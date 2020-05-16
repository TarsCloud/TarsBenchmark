#ifndef _NodeImp_H_
#define _NodeImp_H_

#include "servant/Application.h"
#include "util/tc_thread_mutex.h"
#include "util/tc_thread_pool.h"
#include "BenchmarkThread.h"
#include "transport.h"
#include "commdefs.h"
#include "Node.h"

using namespace bm;
using namespace tars;

/**
 *
 *
 */
class NodeImp : public Node
{
public:
    /**
     *
     */
    virtual ~NodeImp() {}

    /**
     *
     */
    virtual void initialize();

    /**
     *
     */
    virtual void destroy();

    /**
     * @brief  启动压测接口
     *
     */
    int startup(const TaskConf& req, TarsCurrentPtr curr);

    /**
     * @brief  查询压测接口
     *
     */
    int query(const TaskConf& req, QueryRsp& rsp, TarsCurrentPtr curr);

    /**
     * @brief  关停压测接口
     * 
     *
     */
    int shutdown(const TaskConf& req, QueryRsp& rsp, TarsCurrentPtr curr);

    /**
     * @brief  查询正在当前压测的接口
     *
     */
    int capacity(NodeStat &stats, TarsCurrentPtr curr);
private:
    int                                 _max_speed_per_thread;
    int                                 _total_threads;
    vector<BenchmarkThread *>           _thread_pool;
};
/////////////////////////////////////////////////////
#endif
