#ifndef _ProxyServer_H_
#define _ProxyServer_H_

#include <iostream>

#include "servant/Application.h"
#include "util/tc_thread_mutex.h"
#include "util/tc_thread_pool.h"
#include "transport.h"
#include "commdefs.h"
#include "Proxy.h"
#include "Node.h"

using namespace bm;
using namespace tars;

/**
 *
 **/
class ProxyServer : public Application, public TC_ThreadLock
{
public:
    /**
     *
     **/
    virtual ~ProxyServer() {};

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
     * @param
     * @return int
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

extern ProxyServer g_app;

////////////////////////////////////////////
#endif
