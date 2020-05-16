#include "ProxyServer.h"
#include "ProxyImp.h"

using namespace std;

ProxyServer g_app;

/////////////////////////////////////////////////////////////////
void ProxyServer::initialize()
{
    //initialize application here:
    //...
    addServant<ProxyImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".ProxyObj");

    // 启动一个watch线程，防止压测线程长时间空跑
    _runflag = true;
    _next_scan_time = 0;
    auto fw = std::bind(&ProxyServer::daemon, this);
    _watchdog.init(1);
    _watchdog.start();
    _watchdog.exec(fw);
}
/////////////////////////////////////////////////////////////////
void ProxyServer::destroyApp()
{
    //destroy application here:
    //...

    _runflag = false;
    _watchdog.stop();
    _watchdog.waitForAllDone();
}

inline ResultStat& operator+=(ResultStat& l, ResultStat& r)
{
    uint64_t total_request = l.total_request + r.total_request;
    if (total_request > 0)
    {
        l.p90_time  = (l.p90_time*l.total_request + r.p90_time*r.total_request) / total_request;
        l.p99_time  = (l.p99_time*l.total_request + r.p99_time*r.total_request) / total_request;
        l.p999_time = (l.p999_time*l.total_request + r.p999_time*r.total_request) / total_request;
    }

    for (auto & it : r.ret_map)
    {
        l.ret_map[it.first] += it.second;
    }

    for (auto & it : r.cost_map)
    {
        l.cost_map[it.first] += it.second;
    }

    l.total_request += r.total_request;
    l.succ_request  += r.succ_request;
    l.fail_request  += r.fail_request;
    l.total_time    += r.total_time;
    l.send_bytes    += r.send_bytes;
    l.recv_bytes    += r.recv_bytes;
    l.max_time = std::max<double>(l.max_time, r.max_time);
    l.min_time = std::min<double>(l.min_time, r.min_time);
    return l;
}

/////////////////////////////////////////////////////////////////
void ProxyServer::daemon()
{
    while (_runflag)
    {
        int ret_code = 0;
        int err_code = 0;
        string err_msg("");

        Int64 f_start = TNOWMS;
        Int64 cur_time = f_start / 1000;
        PROC_TRY_BEGIN

        // 扫描机器列表
        scanActiveNode(cur_time);

        // 扫描任务列表
        for (auto it = _summary.task.begin(); it != _summary.task.end(); )
        {
            switch (it->second.state)
            {
                case TS_IDLE:
                {
                    map<string, int> speed_quota;
                    TaskConf tconf = it->second.conf;
                    Int32 left_speed  = tconf.speed * tconf.endpoints.size();
                    Int32 totol_speed = tconf.speed * tconf.endpoints.size();
                    for(auto itt = _summary.nodes.begin(); itt != _summary.nodes.end() && left_speed > 0; itt++)
                    {
                        Int32 cost_speed = std::min(itt->second.left_speed, left_speed);
                        tconf.links = it->second.conf.links * cost_speed / totol_speed;
                        tconf.speed = cost_speed / tconf.endpoints.size();
                        if (tconf.speed > 0 && tconf.links > 0)
                        {
                            _next_scan_time = cur_time + 3;
                            startupNodeTask(itt->first, tconf);

                            left_speed -= cost_speed;
                            itt->second.left_speed -= cost_speed;
                            speed_quota[itt->first] = cost_speed;
                        }
                    }

                    Lock lock(*this);
                    it->second.state = TS_RUNNING;
                    it->second.start_time = cur_time;
                    it->second.fetch_time = cur_time;
                    it->second.speed_quota = speed_quota;
                    _summary.result[it->first].time_stamp = cur_time;
                    _summary.total_result[it->first].time_stamp = cur_time;
                    break;
                }
                case TS_RUNNING:
                {
                    // 采集任务状态
                    for (auto & item : it->second.speed_quota)
                    {
                        ResultStat stat;
                        int ret = queryNodeTask(item.first, it->second.conf, stat);
                        if (ret == 0)
                        {
                            Lock lock(*this);
                            _summary.result[it->first] += stat;
                            _summary.total_result[it->first] += stat;
                        }
                    }

                    // 超时退出机制
                    int left_time = it->second.duration == 0 ? (it->second.fetch_time + 300 - cur_time) :
                                    (it->second.start_time + it->second.duration - cur_time);
                    if (left_time < 0)
                    {
                        Lock lock(*this);
                        it->second.state = TS_FINISHED;
                    }
                    break;
                }
                default:
                {
                    // 执行关闭逻辑
                    _next_scan_time = cur_time + 3;
                    for (auto & item : it->second.speed_quota)
                    {
                        shutdownNodeTask(item.first, it->second.conf);
                    }

                    Lock lock(*this);
                    _summary.total_result.erase(it->first);
                    _summary.result.erase(it->first);
                    it = _summary.task.erase(it);
                    continue;
                }
            }
            it++;
        }

        PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

        if (ret_code != 0)
        {
            FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|"  << endl;
        }

        if ((TNOWMS - f_start) < 1000)
        {
            usleep(1000 * (1000 + f_start - TNOWMS));
        }
    }

    FDLOG(__FUNCTION__) << "thread exit" << endl;
}

void ProxyServer::scanActiveNode(long cur_time, bool refresh)
{
    try
    {
        if (cur_time >= _next_scan_time || refresh)
        {
            _next_scan_time = cur_time + 60;

            map<string, NodePrx> nodeprx;
            map<string, NodeStat> nodestat;
            TC_Config &conf = Application::getConfig();
            string node_obj = conf.get("/benchmark<nodeObj>", "benchmark.NodeServer.NodeObj");
            vector<TC_Endpoint> eps = Application::getCommunicator()->getEndpoint4All(node_obj);
            for (size_t i = 0; i < eps.size(); i++)
            {
                NodePrx prx;  NodeStat stat;
                string obj_name = node_obj + "@" + eps[i].toString();
                Application::getCommunicator()->stringToProxy<NodePrx>(obj_name, prx);

                prx->capacity(stat);
                nodeprx[eps[i].getHost()]  = prx;
                nodestat[eps[i].getHost()] = stat;
            }

            Lock lock(*this);
            _nodeprx.swap(nodeprx);
            _summary.nodes = nodestat;
        }

        // 清除非法运行的压测实例
        for (auto &it : _summary.nodes)
        {
            for(auto ite = it.second.executors.begin(); ite != it.second.executors.end();)
            {
                string main_key = ite->servant + "." + ite->rpcfunc;
                if (_summary.task.find(main_key) == _summary.task.end())
                {
                    // 关闭配置
                    TaskConf task;
                    task.servant = ite->servant;
                    task.rpcfunc = ite->rpcfunc;
                    shutdownNodeTask(it.first, task);

                    Lock lock(*this);
                    ite = it.second.executors.erase(ite);
                    continue;
                }
                ++ite;
            }
        }
    }
    catch (TC_Exception& e)
    {
        FDLOG("error") << "exception:" << e.what() << endl;
        throw runtime_error("taf::TC_Exception");
    }
    catch (...)
    {
        FDLOG("error") << "unknown exception" << endl;
    }
}

////////////////////////////////////////////////////////////////
int ProxyServer::startupNodeTask(const string& ipaddr, const TaskConf& task)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (_nodeprx.find(ipaddr) == _nodeprx.end())
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_NOTFIND, err_code, 0, err_msg, "prx not find")
    }

    PROC_NE_EXIT(_nodeprx[ipaddr]->startup(task), 0, ret_code, BM_PROXY_ERR_STARTUP)
    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << ipaddr << "|" << logTars(task) << endl;

    return ret_code;
}

////////////////////////////////////////////////////////////////
int ProxyServer::shutdownNodeTask(const string& ipaddr, const TaskConf& task)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (_nodeprx.find(ipaddr) == _nodeprx.end())
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_NOTFIND, err_code, 0, err_msg, "prx not find")
    }

    QueryRsp rsp;
    PROC_NE_EXIT(_nodeprx[ipaddr]->shutdown(task, rsp), 0, ret_code, BM_PROXY_ERR_SHUTDOWN)
    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << ipaddr << "|" << logTars(task) << endl;

    return ret_code;
}

////////////////////////////////////////////////////////////////
int ProxyServer::queryNodeTask(const string& ipaddr, const TaskConf& task, ResultStat& stat)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    QueryRsp rsp;

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (_nodeprx.find(ipaddr) == _nodeprx.end())
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_NOTFIND, err_code, 0, err_msg, "prx not find")
    }

    PROC_NE_EXIT(_nodeprx[ipaddr]->query(task, rsp), 0, ret_code, BM_PROXY_ERR_SHUTDOWN)
    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)
    stat = rsp.stat;

    TLOGDEBUG((TNOWMS - f_start)  << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << ipaddr << "|task:" << logTars(task) << "|stat:" << logTars(stat) << endl);

    return ret_code;
}

////////////////////////////////////////////////////////////////
bool ProxyServer::getResult(const string &key, ResultStat& stat)
{
    Lock lock(*this);
    auto task = _summary.task.find(key);
    auto result = _summary.result.find(key);
    if (task != _summary.task.end() && result != _summary.result.end())
    {
        stat = result->second;
        int duration = TNOW - task->second.fetch_time;
        stat.avg_speed = duration <= 0 ? 0 : stat.total_request / duration;
        result->second.resetDefautlt();
        result->second.ret_map.clear();
        result->second.cost_map.clear();
        result->second.time_stamp = TNOW;
        task->second.fetch_time = TNOW;
        return true;
    }
    return false;
}

void ProxyServer::getSummary(BenchmarkSummary& summary)
{
    Lock lock(*this);
    summary = _summary;
}

void ProxyServer::updateTask(const string &key, const TaskStat& task)
{
    Lock lock(*this);
    _summary.task[key] = task;
}

/////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    try
    {
        g_app.main(argc, argv);
        g_app.waitForShutdown();
    }
    catch (std::exception& e)
    {
        cerr << "std::exception:" << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }
    return -1;
}
/////////////////////////////////////////////////////////////////
