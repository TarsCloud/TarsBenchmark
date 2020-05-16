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
#include "BenchmarkThread.h"

//////////////////////////////////////////////////////////////////////////
BenchmarkThread::~BenchmarkThread()
{
    if (isAlive())
    {
        terminate();
        getThreadControl().join();
    }

    if (_monitor != NULL)
    {
        delete _monitor;
        _monitor = NULL;
    }
}

void BenchmarkThread::terminate()
{
    _taskconf.resetDefautlt();
    _terminate = true;
}

void BenchmarkThread::init(int shm_key, int shm_size)
{
    _monitor = new Monitor;
    _monitor->initialize(shm_key, shm_size);
    _taskconf.resetDefautlt();

    // 线程启动
    start();
}

void BenchmarkThread::run()
{
    while (!_terminate)
    {
        usleep(1000);
        do_task();
    }
}

void BenchmarkThread::do_task()
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    vector<Transport*> conn_list;

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    TaskConf taskconf = _taskconf;
    size_t connections = taskconf.endpoints.size() * taskconf.links;
    if (taskconf.runflag == 0 || connections == 0)
    {
        PROC_TRY_EXIT(ret_code, taskconf.runflag, err_code, connections, err_msg, "task not conf")
    }

    /*******************************/
    /* 先执行一段初始化代码，然后干活 */
    /*******************************/
    TC_Epoller eloop;
    eloop.create(MAX_FD);

    // 协议初始化
    tarsProtocol proto;
    proto._servant  = taskconf.servant;
    proto._function = taskconf.rpcfunc;
    proto._paraList = taskconf.paralist;
    proto._paraVals = taskconf.paravals;

    // 创建压测连接
    for (size_t i = 0; i < taskconf.endpoints.size(); i++)
    {
        TC_Endpoint ep;
        ep.parse(taskconf.endpoints[i]);
        for (int ii = 0; ii < taskconf.links; ii++)
        {
            Transport* conn = ep.isTcp() ? (Transport*)(new TCPTransport(ep, &eloop))
                                         : (Transport*)(new UDPTransport(ep, &eloop));
            conn->initialize(_monitor, &proto);
            conn_list.push_back(conn);
        }
        proto._timeOut  = ep.getTimeout();
    }

    // 清理缓存区数据, 准备启动压测
    getMonitor()->clear();
    int64_t next_send_time = 0;
    static std::atomic<int64_t> seq_num{0};
    int64_t interval = taskconf.speed == 0 ? 0 : 1000000 * int64_t(taskconf.links) / taskconf.speed;
    while (true)
    {
        seq_num = seq_num < 0 ? 1 : ++seq_num;
        int64_t curr_send_time = TC_Common::now2us();
        if (curr_send_time >= next_send_time)
        {
            next_send_time = curr_send_time + interval;
            for (size_t i = 0; i < conn_list.size(); i++)
            {
                conn_list[i]->trySend(seq_num);
                conn_list[i]->checkTimeOut(curr_send_time/1000);
            }
        }

        Transport::handle(&eloop, 1);
        getMonitor()->syncStat(curr_send_time/1000);

        // 比较一下配置是否发生变更
        if (taskconf != _taskconf)
        {
            PROC_TRY_EXIT(ret_code, taskconf.runflag, err_code, _taskconf.runflag, err_msg, "exit benchmark")
        }
    }

    // 析构链接前收发一次
    Transport::handle(&eloop, 1000);
    getMonitor()->syncStat(0);

    PROC_TRY_END(err_msg, ret_code, -1, -1)

    // 析构链接
    for (size_t i = 0; i < conn_list.size(); i++)
    {
        delete conn_list[i];
    }

    if (ret_code != 0)
    {
        FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << logTars(_taskconf) << endl;
    }
    conn_list.clear();
}
