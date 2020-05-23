#include "AdminImp.h"
#include "AdminServer.h"
#include "tars_protocol.h"
#include "json_protocol.h"
#include "servant/Application.h"

using namespace std;
extern AdminServer g_app;

#define DELETE_POINT(p) if (p != NULL) { delete p; p = NULL;}

//////////////////////////////////////////////////////
void AdminImp::initialize()
{
    //initialize servant here:
    //...
}

//////////////////////////////////////////////////////
void AdminImp::destroy()
{
    //destroy servant here:
    //...
    _factory.destroyObject();
}

int AdminImp::check(const BenchmarkUnit& req, TaskConf &conf, int timeout)
{
    try
    {
        conf.proto = req.proto;
        conf.endpoints = req.endpoints;
        Protocol *proto = _factory.get(req.proto + "Protocol");
        if (req.proto == "json" || req.proto == "tars")
        {
            if (req.servant.empty() || req.rpcfunc.empty())
            {
                return BM_SERVER_ERR_PARAM;
            }
            conf.service = req.servant + ":" + req.rpcfunc;
            conf.paralist.push_back(req.servant);
            conf.paralist.push_back(req.rpcfunc);
            conf.paralist.push_back(TC_Common::tostr(timeout));
            conf.paralist.push_back(req.para_input);
            conf.paralist.push_back(req.para_value);
        }
        else if (req.proto == "http")
        {
            if (req.paralist.size() != 4)
            {
                return BM_SERVER_ERR_PARAM;
            }
            conf.service  = req.paralist[0];
            conf.paralist = req.paralist;
        }

        if (timeout != 0)
        {
            return proto->initialize(conf.paralist);
        }
    }
    catch (exception& e)
    {
        FDLOG("error") << __FUNCTION__ << "|" << e.what() << endl;
        return BM_ADMIN_ERR_PROTO;
    }
    return BM_SUCC;
}

int AdminImp::startup(const BenchmarkUnit& req, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    TC_Endpoint ep;
    bool has_bad_endpoint = false;
    for (size_t i = 0; i < req.endpoints.size(); i++)
    {
        try { 
            ep.parse(req.endpoints[i]);
            if (ep.getPort() == 0 || ep.getPort() > 65535)
            {
                has_bad_endpoint = true;
            }
        }
        catch (TC_Exception& e)
        {
            TLOGTARS("parse endpoint fail:" << e.what() << ", " << req.endpoints[i] << endl);
            has_bad_endpoint = true;
        }
    }

    if (has_bad_endpoint || req.endpoints.size() == 0)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_ENDPOINT, err_code, req.endpoints.size(), err_msg, "err endpoint")
    }

    TaskStat task;
    int ret = check(req, task.conf, ep.getTimeout());
    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, ret, err_code, 0, err_msg, "check param")
    }

    if ((req.speed % req.links != 0) || (req.speed / req.links) > 1000)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_CONNECTION, err_code, 0, err_msg, "link and speed not match")
    }

    BenchmarkSummary summary;
    g_app.getSummary(summary);
    string main_key = GetMainKey(task.conf);
    if (summary.task.find(main_key) != summary.task.end())
    {
        PROC_TRY_EXIT(ret_code, BM_ADMIN_ERR_TASK, err_code, 0, err_msg, "task is running")
    }

    int64_t totol_left_speed = 0;
    int64_t need_speed = req.speed * req.endpoints.size();
    for (auto &node : summary.nodes)
    {
        totol_left_speed += node.second.left_speed;
    }

    if (need_speed < 0 || totol_left_speed < need_speed)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_RESOURCE, err_code, totol_left_speed, err_msg, "need more nodes")
    }

    // 处理入参配置
    task.state = TS_IDLE;
    task.duration     = req.duration;
    task.conf.speed   = req.speed;
    task.conf.links   = req.links;
    g_app.updateTask(main_key, task);

    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG("info") << __FUNCTION__ << "|" << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << logTars(req) << "|" << curr->getIp() << endl;

    return ret_code;
}

int AdminImp::query(const BenchmarkUnit& req, ResultStat& stat, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    TaskConf tconf;
    int ret = check(req, tconf);
    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, ret, err_code, 0, err_msg, "check param")
    }

    string main_key = GetMainKey(tconf);
    if (!g_app.getResult(main_key, stat))
    {
        PROC_TRY_EXIT(ret_code, BM_ADMIN_ERR_NOTFIND, err_code, 0, err_msg, "not find interface")
    }

    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    if (ret_code != 0)
    {
        FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << logTars(req) << "|" << curr->getIp() << endl;
    }
    else
    {
        TLOGDEBUG((TNOWMS - f_start) << "|" << "|req:" << logTars(req) << "|" << logTars(stat) << "|" << curr->getIp() << endl << endl);
    }
    return ret_code;
}

int AdminImp::shutdown(const BenchmarkUnit& req, ResultStat& stat, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    TaskConf tconf;
    int ret = check(req, tconf);
    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, ret, err_code, 0, err_msg, "check param")
    }

    BenchmarkSummary summary;
    g_app.getSummary(summary);
    string main_key = GetMainKey(tconf);
    auto task = summary.task.find(main_key);
    if (task == summary.task.end() || task->second.state != TS_RUNNING)
    {
        PROC_TRY_EXIT(ret_code, BM_ADMIN_ERR_RUNNING, err_code, 0, err_msg, "task not running")
    }

    // 移交线程去执行关闭策略
    summary.task[main_key].state = TS_FINISHED;
    g_app.updateTask(main_key, summary.task[main_key]);

    // 统计最终结果输出
    if (summary.total_result.find(main_key) != summary.total_result.end())
    {
        stat = summary.total_result[main_key];
        int duration = TNOWMS/1000 - summary.task[main_key].start_time;
        stat.avg_speed = duration <= 0 ? 0 : stat.total_request / duration;
    }

    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG("info") << __FUNCTION__ << "|" << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|req:" << logTars(req) << "|" << logTars(stat) << "|" << curr->getIp() << endl;

    return ret_code;
}

int AdminImp::test(const BenchmarkUnit& req, string& rsp, string& errmsg, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    char *sendbuf = NULL;
    char *recvbuf = NULL;

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (req.proto != "tars" && req.proto != "json")
    {
        PROC_TRY_EXIT(ret_code, BM_ADMIN_ERR_PROTO, err_code, 0, errmsg, "protocol don't support")
    }

    if (req.endpoints.size() == 0)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_ENDPOINT, err_code, req.endpoints.size(), errmsg, "err endpoint")
    }

    TaskConf tconf;
    TC_Endpoint ep;
    ep.parse(req.endpoints[0]);
    int ret = check(req, tconf, ep.getTimeout());
    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, ret, err_code, 0, err_msg, "check param")
    }

    Protocol *proto = _factory.get(req.proto + "Protocol");
    proto->initialize(tconf.paralist);

    int seq = 1;
    TC_Config &conf = Application::getConfig();
    int sendlen = TC_Common::strto<int>(conf.get("/benchmark<sendSize>", "4194304"));
    size_t recvlen = TC_Common::strto<int>(conf.get("/benchmark<recvSize>", "8388608"));
    sendbuf = new char[sendlen];
    recvbuf = new char[recvlen];
    ret = proto->encode(sendbuf, sendlen, seq);
    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, BM_ADMIN_ERR_ENCODE, err_code, ret, errmsg, "encode fail")
    }

    if (ep.isTcp())
    {
        TC_TCPClient client(ep.getHost(), ep.getPort(), ep.getTimeout());
        ret = client.sendRecv(sendbuf, (size_t)sendlen, recvbuf, recvlen);
    }
    else
    {
        TC_TCPClient client(ep.getHost(), ep.getPort(), ep.getTimeout());
        ret = client.sendRecv(sendbuf, (size_t)sendlen, recvbuf, recvlen);
    }

    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, BM_ADMIN_ERR_SOCKET, err_code, ret, errmsg, "socket sendrecv fail")
    }

    TarsInputStream<BufferReader> is, isf;
    is.setBuffer(recvbuf + 4, recvlen - 4);

    ResponsePacket res;
    res.readFrom(is);
    isf.setBuffer(res.sBuffer);
    ret_code = res.iRet;
    if (ret_code == 0)
    {
        size_t n = 0;
        uint8_t head_type, head_tag;
        TarsPeekFromHead(isf, head_type, head_tag, n);
        if (head_tag == 0 && head_type < 32)
        {
            isf.read(ret_code, head_tag, true);
        }

        if (req.paralist.size() == 0)
        {
            PROC_TRY_EXIT(ret_code, ret_code, err_code, ret, err_msg, "has no output")
        }

        // 对应答包进行解码输出
        if (req.proto == "tars" && req.paralist.size() > 0)
        {
            tarsProtocol *p = (tarsProtocol *)proto;
            vector<string> vi = TC_Common::sepstr<string>(req.para_input, "|");
            vector<string> vo = TC_Common::sepstr<string>(req.paralist[0], "|");
            for (size_t i = 0; i < vo.size(); i++)
            {
                rsp += p->decode(isf, vo[i], int(i+vi.size()+1), false) + "<br>";
            }
        }
        else
        {
            vector<JsonField> fields;
            JsonValueObjPtr out = new JsonValueObj;
            jsonProtocol *p = (jsonProtocol *)proto;
            p->parseField(JsonValueObjPtr::dynamicCast(TC_Json::getValue(req.paralist[0])), fields);
            for(auto & field : fields) 
            {
                out->value[field.name] = p->decode(isf, field);
            }
            rsp = TC_Json::writeValue(out);
        }
    }

    PROC_TRY_END(errmsg, ret_code, BM_ADMIN_ERR_DECODE, BM_ADMIN_ERR_DECODE)
    DELETE_POINT(sendbuf)
    DELETE_POINT(recvbuf)

    FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << errmsg << "|" << "|req:" << logTars(req) << "|out:" << rsp << "|" << curr->getIp() << endl;

    return ret_code;
}


