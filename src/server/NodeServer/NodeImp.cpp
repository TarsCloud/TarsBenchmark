#include "NodeImp.h"
#include "servant/Application.h"

using namespace std;

//////////////////////////////////////////////////////
void NodeImp::initialize()
{
    //initialize servant here:
    //...
    try
    {
        static atomic<int> node_count{0};
        if (++node_count > 1)
        {
            throw TC_Exception("imp thread must equal one");
        }

        TC_Config &conf = Application::getConfig();
        _total_threads = TC_Common::strto<int>(conf.get("/benchmark<totalThreads>", TC_Common::tostr(getProcNum())));
        _max_speed_per_thread = TC_Common::strto<int>(conf.get("/benchmark<maxThreadSpeed>", "25000"));
        int shm_base = TC_Common::strto<int>(conf.get("/benchmark<shmBase>", "202005010"));
        int shm_size = TC_Common::strto<int>(conf.get("/benchmark<shmSize>", "1048576"));
        for (int i = 0; i < _total_threads; i++)
        {
            BenchmarkThread *thread = new BenchmarkThread;
            thread->init(shm_base + 1, shm_size);
            _thread_pool.push_back(thread);
        }
    }
    catch (TC_Exception& e)
    {
        TLOGERROR("tars exception:" << e.what() << endl);
        throw TC_Exception(e.what());
    }
    catch (exception& e)
    {
        TLOGERROR("std exception:" << e.what() << endl);
        throw TC_Exception(e.what());
    }
    catch (...)
    {
        TLOGERROR("unknow exception" << endl);
        throw TC_Exception("unknow");
    }
}

//////////////////////////////////////////////////////
void NodeImp::destroy()
{
    //destroy servant here:
    //...
    for (size_t i = 0; i < _thread_pool.size(); ++i)
    {
        _thread_pool[i]->terminate();
        delete _thread_pool[i];
    }
    _thread_pool.clear();

}

int NodeImp::startup(const TaskConf& req, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    int left_speed = req.speed * req.endpoints.size();
    if (req.service.empty() || req.proto.empty() || left_speed < 0)
    {
        PROC_TRY_EXIT(ret_code, BM_SERVER_ERR_PARAM, err_code, 0, err_msg, "check param")
    }

    if ((req.speed % req.links != 0) || (req.speed / req.links) > 1000)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_CASEMATCH, err_code, 0, err_msg, "link and speed not match")
    }

    int run_thread = 0, left_thread = 0;
    for (size_t i = 0; i < _thread_pool.size(); i++)
    {
        TaskConf& tconf = _thread_pool[i]->getTaskConf();
        if (tconf.runflag == TS_IDLE)
        {
            left_thread++;
            continue;
        }

        if (tconf >= req && tconf <= req && tconf.runflag == TS_RUNNING)
        {
            run_thread++;
        }
    }

    if (run_thread > 0)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_RUNNING, err_code, run_thread, err_msg, "repeate")
    }

    int need_thread = (req.speed * req.endpoints.size() + _max_speed_per_thread - 1) / _max_speed_per_thread;
    if (need_thread > left_thread)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_RESOURCE, err_code, left_thread, err_msg, "resource busy")
    }

    // 改写配置准备启动压测
    for (size_t i = 0; i < _thread_pool.size() && left_speed > 0; i++)
    {
        TaskConf& tconf = _thread_pool[i]->getTaskConf();
        if (tconf.runflag == TS_IDLE)
        {
            int threadspeed = std::min(left_speed, _max_speed_per_thread);
            tconf.proto = req.proto;
            tconf.service = req.service;
            tconf.paralist = req.paralist;       // 压测的入参配置
            tconf.endpoints = req.endpoints;     // 压测目标服务器
            tconf.links = req.links / need_thread;   // 压测目标服务器
            tconf.speed = threadspeed / req.endpoints.size();
            tconf.runflag = TS_RUNNING;
            left_speed -= threadspeed;
        }
    }

    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG("info") << __FUNCTION__ << "|" << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg
                  << "|req:" << logTars(req) << "|" << curr->getIp()<< endl;
    return ret_code;
}

int NodeImp::shutdown(const TaskConf& req, QueryRsp& rsp, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (req.service.empty() || req.proto.empty())
    {
        PROC_TRY_EXIT(ret_code, BM_SERVER_ERR_PARAM, err_code, 0, err_msg, "check param")
    }

    // 停止前把数据搜集上来
    this->query(req, rsp, curr);
    for(size_t i = 0; i < _thread_pool.size(); i++)
    {
        TaskConf& tconf = _thread_pool[i]->getTaskConf();
        if (tconf >= req && tconf <= req)
        {
            tconf.resetDefautlt();
        }
    }

    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG("info") << __FUNCTION__ << "|" << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg
                  << "|req:" << logTars(req) << "|rsp:" << logTars(rsp) << "|" << curr->getIp()<< endl;
    return ret_code;
}

int NodeImp::query(const TaskConf& req, QueryRsp& rsp, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (req.service.empty() || req.proto.empty())
    {
        PROC_TRY_EXIT(ret_code, BM_SERVER_ERR_PARAM, err_code, 0, err_msg, "check param")
    }

    IntfStat stat_final;
    map<int, int> ret_final;
    for(size_t i = 0; i < _thread_pool.size(); i++)
    {
        TaskConf tconf = _thread_pool[i]->getTaskConf();
        if (tconf >= req && tconf <= req)
        {
            /* 判断是同一个servant和RPC接口 */
            vector<IntfStat> stat_list;
            if (_thread_pool[i]->getMonitor()->fetch(stat_list))
            {
                for (size_t ii = 0; ii < stat_list.size(); ii++)
                {
                    map<int, int> ret_val = str2map(string((char *)stat_list[ii].retCount));
                    for(auto &itm : ret_val)
                    {
                        ret_final[itm.first] += itm.second;
                    }
                    stat_final += stat_list[ii];
                }
            }
        }
    }

    rsp.ipaddr = ServerConfig::LocalIp;
    // rsp.stat.time_stamp    = TNOW;
    rsp.stat.ret_map       = ret_final;
    rsp.stat.total_request = stat_final.totalCount;
    rsp.stat.succ_request  = stat_final.succCount;
    rsp.stat.fail_request  = stat_final.failCount;
    rsp.stat.max_time   = stat_final.maxTime;
    rsp.stat.min_time   = stat_final.minTime;
    rsp.stat.total_time = stat_final.totalTime;
    rsp.stat.p999_time  = stat_final.p999Time;
    rsp.stat.p99_time   = stat_final.p99Time;
    rsp.stat.p90_time   = stat_final.p90Time;
    rsp.stat.send_bytes = stat_final.totalSendBytes;
    rsp.stat.recv_bytes = stat_final.totalRecvBytes;
    for (size_t i = 0; i < MAX_STEP_COST; i++)
    {
        rsp.stat.cost_map[i] = stat_final.costTimes[i];
    }
    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    if (ret_code != 0)
    {
        FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|req:" << logTars(req) << "|rsp:" << logTars(rsp) << endl;
    }
    else
    {
        TLOGDEBUG((TNOWMS - f_start) << "|req:" << logTars(req) << "|rsp:" << logTars(rsp) << endl);
    }
    return ret_code;
}

int NodeImp::capacity(NodeStat &stats, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    int cur_thread = 0;
    map<string, TaskConf> cur_task;
    stats.ipaddr = ServerConfig::LocalIp;
    stats.max_speed = _total_threads * _max_speed_per_thread;
    stats.max_threads = _total_threads;
    for(size_t i = 0; i < _thread_pool.size(); i++)
    {
        TaskConf tconf = _thread_pool[i]->getTaskConf();
        if (tconf.runflag)
        {
            cur_thread += 1;
            string main_key = tconf.proto + "." + tconf.service;
            if (cur_task.find(main_key) == cur_task.end())
            {
                cur_task[main_key] = tconf;
            }
            else
            {
                cur_task[main_key].speed += tconf.speed;
                cur_task[main_key].links += tconf.links;
            }
        }
    }

    for (auto &itm : cur_task)
    {
        ExecItem runner;
        runner.proto   = itm.second.proto;
        runner.service = itm.second.service;
        runner.speed   = itm.second.speed * itm.second.endpoints.size();
        runner.links   = itm.second.links;
        runner.threads = (_max_speed_per_thread + itm.second.speed - 1) / _max_speed_per_thread;
        stats.executors.push_back(runner);
    }

    stats.left_threads = _total_threads - cur_thread;
    stats.left_speed   = stats.left_threads * _max_speed_per_thread;

    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    if (ret_code != 0)
    {
        FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << logTars(stats) << endl;
    }
    else
    {
        TLOGDEBUG((TNOWMS - f_start) << "|" << logTars(stats) << endl);
    }
    return ret_code;
}

