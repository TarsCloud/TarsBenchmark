#ifndef _ADMIN_SERVER_H_
#define _ADMIN_SERVER_H_

#include <iostream>

#include "servant/Application.h"
#include "util/tc_thread_mutex.h"
#include "util/tc_thread_pool.h"
#include "transport.h"
#include "commdefs.h"
#include "Admin.h"
#include "Node.h"

using namespace bm;
using namespace tars;

/**
 * @brief 生成主key
 *
 * @param l  tconf   任务配置
 *
 * @return string
 */
inline string GetMainKey(const TaskConf& tconf)
{
    ostringstream osk;
    osk << tconf.proto << "." << tconf.service;
    return osk.str();
}

/**
 * @brief 重置ResultStat
 *
 * @param rs  目标结构
 * @param cur_time  当前时间
 */
inline ResultStat& resetStat(ResultStat& rs, Int64 cur_time = TNOW)
{
    rs.resetDefautlt();
    rs.ret_map.clear();
    rs.cost_map.clear();
    rs.time_stamp = cur_time;
    return rs;
}

/**
 * @brief 计算速率
 *
 * @param rs        目标结构
 * @param cur_time  当前时间
 */
inline Int64 calcSpeed(ResultStat& rs, Int64 cur_time = TNOW)
{
    Int64 duration = cur_time - rs.time_stamp;
    return duration <= 0 ? 0 : rs.total_request / duration;
}

/**
 * @brief 重载ResultStat结构+=符号
 *
 * @param l  左边的rs
 * @param r  右边的rs
 */
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
/**
 *
 **/
class AdminServer : public Application, public TC_ThreadLock
{
public:
    /**
     *
     **/
    virtual ~AdminServer() {};

    /**
     *
     **/
    virtual void initialize();

    /**
     *
     **/
    virtual void destroyApp();

    /**
     * 看护线程入口
     *
     **/
    void daemon();

    /**
     * @brief 更新task配置
     *
     * @param key       主key
     * @param task      任务配置
     */
    void updateTask(const string &key, const TaskStat& task);

    /**
     * @brief 获取状态总入口
     *
     * @param summary   状态
     */
    void getSummary(BenchmarkSummary& summary);

    /**
     * @brief 函数注释
     *
     * @param  key      主键
     * @param  stat     状态
     *
     * @return bool
     */
    bool getResult(const string &key, ResultStat& stat);

    /**
     * @brief 扫描当前处在活动状态的node
     *
     * @param cur_time  当前时间，单位为秒
     * @param refresh   强刷标志
     */
    void scanActiveNode(long cur_time, bool refresh = false);

    /**
     * @brief 启动node上的任务
     *
     * @param ipaddr   IP地址
     * @param task     任务配置
     *
     * @return int
     */
    int startupNodeTask(const string& ipaddr, const TaskConf& task);

    /**
     * @brief 关闭node上某个任务
     *
     * @param ipaddr   IP地址
     * @param task     任务配置
     *
     * @return int
     */
    int shutdownNodeTask(const string& ipaddr, const TaskConf& task);

    /**
     * @brief 关闭node上某个任务
     *
     * @param ipaddr   IP地址
     * @param task     任务配置
     * @param stat     状态
     *
     * @return int
     */
    int queryNodeTask(const string& ipaddr, const TaskConf& task, ResultStat& stat);
private:
    int64_t                 _next_scan_time;      // 下一次扫描时间
    bool                    _runflag = true;      // 看门狗运行标志
    TC_ThreadPool           _watchdog;            // 看门狗线程
    BenchmarkSummary        _summary;             // 状态统计
    map<string, NodePrx>    _nodeprx;             // 以机器IP为key
};

extern AdminServer g_app;

////////////////////////////////////////////
#endif
