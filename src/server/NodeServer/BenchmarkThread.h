/**
* Tencent is pleased to support the open source community by making DCache available.
* Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
* Licensed under the BSD 3-Clause License (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of the License at
*
* https://opensource.org/licenses/BSD-3-Clause
*
* Unless required by applicable law or agreed to in writing, software distributed under
* the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
* either express or implied. See the License for the specific language governing permissions
* and limitations under the License.
*/
#ifndef __BENCHMARK_THREAD_H_
#define __BENCHMARK_THREAD_H_

#include "Node.h"
#include "commdefs.h"
#include "transport.h"
#include "tars_protocol.h"
#include "util/tc_thread.h"
#include "servant/Application.h"

using namespace bm;
using namespace tars;

class BenchmarkThread : public TC_Thread
{
public:
    /**
     * 构造函数
     */
    BenchmarkThread(): _terminate(false), _monitor(NULL) {}

    /**
     * 析构函数
     */
    ~BenchmarkThread();

    /**
     * @brief 函数注释
     *
     * @param  shm_key       监控器的共享内存KEY
     * @param  shm_size      监控器的共享内存长度
     */
    void init(int shm_key, int shm_size);

    /**
     * @brief 获取当前线程的监控器
     *
     * @return Monitor
     */
    Monitor* getMonitor() { return _monitor; }

    /**
     * @brief 获取当前任务配置
     *
     * @return TaskConf
     */
    TaskConf& getTaskConf() { return _taskconf; }


    /**
     * @brief 轮询函数
     *
     */
    virtual void run();

    /**
     * @brief 结束线程
     *
     */
    void terminate();
protected:
    /**
     * @brief 执行压测
     *
     */
    void do_task();

private:
    bool        _terminate;         // 线程结束标志
    Monitor*    _monitor;           // 监控器
    TaskConf    _taskconf;          // 线程安全考虑
};

#endif
