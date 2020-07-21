/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */
#include "monitor.h"
#include "transport.h"
#include "signal.h"
#include <sys/wait.h>
using namespace bm;

// LICOTE命令声明
LICOTE_OPTION_BEGIN
LICOTE_OPTION_DECL("-c", NULL, "number of connections");
LICOTE_OPTION_DECL("-D", NULL, "target server address(ipv4)");
LICOTE_OPTION_DECL("-P", NULL, "target server port");
LICOTE_OPTION_DECL("-T", "o", "network protocol(tcp|udp)");
LICOTE_OPTION_DECL("-I", "o", "continue time(by second)");
LICOTE_OPTION_DECL("-i", "o", "view interval");
LICOTE_OPTION_DECL("-t", "o", "overtime time");
LICOTE_OPTION_DECL("-s", "o", "maximum tps");
LICOTE_OPTION_DECL("-n", "o", "maximum process");
LICOTE_OPTION_DECL("-p", "o", "server protocol(tars|json|http)");
LICOTE_OPTION_DECL("-h", "h", "help info");

LICOTE_SET_LANGUAGE(CN);
LICOTE_SET_VERSION("BSD");
LICOTE_SET_EXAMPLE("./tb -c 2000 -s 8000 -D 192.168.16.1 -P 10505 -p json -i 10 -S tars.DemoServer.DemoObj -M test -C test;"
                   "./tb -c 2000 -s 8000 -t 6000 -D 192.168.31.1,192.168.32.1 -P 80 -u http://www.qq.com");
LICOTE_SET_DESCRIPTION("tars benchmark tool");
LICOTE_OPTION_END

/**
*  全局变量定义
*/
bool g_run_flag;
size_t g_speed;
size_t g_cons_num;
size_t g_interval;
size_t g_run_cores;
string g_proto_name;
size_t g_begin_time;
IntfStat g_intf_stat;
vector<TC_Endpoint> g_endpoints;

/**
*  初始化函数
*/
int initialize(int argc, char *argv[])
{
    try
    {
        // LICOTE初始化
        if (licote_option_init(argc, argv) != 0)
        {
            return BM_INIT_PARAM;
        }

        // 获取压测核数
        g_run_cores = std::max(LICODE_GETINT("-n", getProcNum()), (int64_t)1);

        // EndPoint初始化
        TC_Endpoint::EType netType = LICODE_GETSTR("-T", "tcp") != "udp" ? TC_Endpoint::TCP : TC_Endpoint::UDP;
        TC_Endpoint ep("", LICODE_GETINT("-P", 0), LICODE_GETINT("-t", 3000), netType);
        vector<string> vd = TC_Common::sepstr<string>(LICODE_GETSTR("-D", ""), ",");
        for (size_t ii = 0; ii < vd.size() && !vd[ii].empty(); ii++)
        {
            ep.setHost(vd[ii]);
            g_endpoints.push_back(ep);
        }

        if (ep.getTimeout() == 0 || ep.getPort() == 0 || g_endpoints.size() == 0)
        {
            licote_option_help("参数格式不正确: 目标服务器配置错误\n");
        }

        // 压测参数初始化
        g_speed = (LICODE_GETINT("-s", 0) + g_run_cores - 1) / g_run_cores;
        g_cons_num = (LICODE_GETINT("-c", 0) + g_run_cores - 1) / g_run_cores;
        if (g_cons_num == 0 || LICODE_GETINT("-i", 10) == 0)
        {
            licote_option_help("参数格式不正确: -i|-c 不能为0\n");
        }
        else if (g_cons_num * g_endpoints.size() > MAX_FD)
        {
            licote_option_help("参数格式不正确: 连接数超过系统限制\n");
        }

        ProtoFactory factory;
        g_proto_name = LICODE_GETSTR("-p", "http") + "Protocol";
        if (factory.get(g_proto_name) == NULL)
        {
            licote_option_help("参数格式不正确: 协议不存在\n");
        }

        factory.get(g_proto_name)->initialize(argc, argv);
        g_interval = g_speed == 0 ? 0 : (1000000 * g_cons_num) / g_speed;
        memset(&g_intf_stat, 0, sizeof(IntfStat));
        Monitor::getInstance()->initialize();
        g_run_flag = true;
    }
    catch (exception &e)
    {
        licote_option_help("系统异常: %s\n", e.what());
    }
    return 0;
}

/**
*  压测主程序run
*/
int run(int seqno, int argc, char *argv[])
{
    // 事件初始化
    TC_Epoller loop;
    loop.create(MAX_FD);

    // 创建连接
    vector<Transport *> connections;
    for (size_t ii = 0; ii < g_cons_num; ii++)
    {
        for (size_t i = 0; i < g_endpoints.size(); i++)
        {
            Transport *conn = g_endpoints[i].isTcp() ? (Transport *)(new TCPTransport(g_endpoints[i], &loop))
                                                     : (Transport *)(new UDPTransport(g_endpoints[i], &loop));
            conn->initialize(Monitor::getInstance(), g_proto_name, argc, argv);
            connections.push_back(conn);
        }
    }

    int64_t req_no = seqno;
    int64_t next_send_time = 0;
    int64_t time_out = LICODE_GETINT("-t", 3000);
    while (g_run_flag)
    {
        try
        {
            int64_t now_us = TC_Common::now2us();
            if (now_us >= next_send_time)
            {
                next_send_time = now_us + Monitor::getInstance()->calcInterval(g_interval, connections.size());
                for (size_t i = 0; i < connections.size(); i++)
                {
                    req_no += g_run_cores;
                    connections[i]->trySend(req_no);
                    connections[i]->checkTimeOut(now_us / 1000);
                }
            }

            Transport::handle(&loop, 1);
            Monitor::getInstance()->syncStat(now_us / 1000, time_out);
        }
        catch (tars::TC_Exception &e)
        {
            cerr << "tars exception:" << e.what() << endl;
        }
    }

    // 析构链接前收发一次
    Transport::handle(&loop, 1000);
    Monitor::getInstance()->syncStat(0, time_out);
    for (size_t i = 0; i < connections.size(); i++)
    {
        delete connections[i];
    }
    connections.clear();
    return 0;
}

/**
*  处理信号退出
*/
void procSignal(int signo)
{
    if (SIGINT == signo || SIGUSR1 == signo || SIGTERM == signo || SIGKILL == signo)
    {
        g_run_flag = false;
    }
}

/**
 *  输出耗时统计
 */
void printCost(const IntfStat &stat)
{
    double total_count = 0.0;
    double meger_count = 0.0;
    static int arrCost[MAX_STEP_COST] = {0, 10, 30, 50, 100, 500, 3000, 5000, 99999, 0};
    for (size_t i = 0; i < MAX_STEP_COST && arrCost[i + 1] != 0; i++)
    {
        total_count += stat.costTimes[i];
    }

    total_count = max(1.0, total_count);
    for (size_t i = 0; i < MAX_STEP_COST && arrCost[i + 1] != 0; i++)
    {
        meger_count += stat.costTimes[i];
        printf("[%5d - %5d ms] %7ld\t%2.2f%%\t\t%2.2f%%\n", arrCost[i], arrCost[i + 1], stat.costTimes[i],
               100 * (double)stat.costTimes[i] / total_count, 100 * meger_count / total_count);
    }
}

/**
 *  输出压测周期结果
 */
int64_t printPeriod(int intvl_time)
{
    IntfStat stat;
    map<int, int> ret_final;
    map<int64_t, int64_t> exec_speed;
    vector<IntfStat> stat_list;
    if (Monitor::getInstance()->fetch(stat_list))
    {
        for (size_t ii = 0; ii < stat_list.size(); ii++)
        {
            map<int, int> ret_map = str2map(string((char *)stat_list[ii].retCount));
            for (map<int, int>::iterator itm = ret_map.begin(); itm != ret_map.end(); itm++)
            {
                ret_final[itm->first] += itm->second;
            }

            if (stat_list[ii].execKey != 0)
            {
                // 统计周期是1s，所以速率近似为(succCount+failCount), 最后一个周期以最大为准
                int64_t exec_key = stat_list[ii].execKey;
                int64_t exec_requests = stat_list[ii].succCount + stat_list[ii].failCount;
                exec_speed[exec_key] = std::max(exec_requests, exec_speed[exec_key]);
            }
            stat += stat_list[ii];
        }
        g_intf_stat += stat;
    }

    if (intvl_time > 0)
    {
        size_t total_requests = stat.succCount + stat.failCount;
        double total_decimal = std::max(1.00, (double)total_requests);
        double fail_rate = min((double)stat.failCount, total_decimal) / total_decimal;
        printf("\n\n--------------------------------------------------------------------------------------------------------------------\n");
        printf("Time\t\t\tTotal\tSucc\tFail\tRate\tMax(ms)\tMin(ms)\tAvg(ms)\tP90(ms)\tP99(ms)\tP999(ms)\tTPS\n");
        printf("%s\t%-6ld\t%-6ld\t%-6ld\t%0.2f%%\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t\t%ld",
               TC_Common::now2str("%Y-%m-%d %H:%M:%S").c_str(),
               stat.totalCount, stat.succCount, stat.failCount, (1 - fail_rate) * 100,
               stat.maxTime, stat.minTime, stat.totalTime / total_decimal,
               stat.p90Time, stat.p99Time, stat.p999Time,
               total_requests / intvl_time);

        printf("\n\n\nCode Details: [%s]\n", map2str(ret_final).c_str());
        printf("--------------------------------------------------------------------------------------------------------------------\n");
        printCost(stat);
    }

    int64_t totol_speed = 0;
    for (map<int64_t, int64_t>::iterator it = exec_speed.begin(); it != exec_speed.end(); it++)
    {
        totol_speed += it->second;
    }
    return totol_speed;
}

/**
 *  输出最终压测结果
 */
void printFinal(int intvl_time)
{
    int64_t final_speed = printPeriod(0);
    string endpoints = g_endpoints[0].toString();
    for (size_t ii = 1; ii < g_endpoints.size(); ii++)
    {
        endpoints.append("\n                        ").append(g_endpoints[ii].toString());
    }

    size_t total_requests = g_intf_stat.succCount + g_intf_stat.failCount;
    double total_decimal = std::max(1.00, (double)total_requests);
    double fail_rate = min((double)g_intf_stat.failCount, total_decimal) / total_decimal;

    int real_intvl_time = intvl_time - LICODE_GETINT("-t", 3000) / 3000;
    printf("\n\n--------------------------------------------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("----------------------------------   Finish Pressure Test   --------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("Server Numbers:         %ld\n", g_endpoints.size());
    printf("Server Endpoint:        %s\n", endpoints.c_str());
    printf("Server Protocol:        %s\n\n\n", LICODE_GETSTR("-p", "").c_str());

    printf("Concurrency Procesor:     %ld\n", g_run_cores);
    printf("Concurrency Connections:  %lu\n", g_cons_num * g_run_cores);
    printf("Connections per Procesor: %ld\n", g_cons_num);
    printf("Success requests:         %ld\n", g_intf_stat.succCount);
    printf("Success rate:             %.2f%%\n", (1 - fail_rate) * 100);
    printf("Failed requests:          %ld\n", g_intf_stat.failCount);
    printf("Total requests:           %ld\n", g_intf_stat.totalCount);
    printf("Total duration:           %d[sec]\n", real_intvl_time);
    printf("Transfer rate:            %.2f[Kbytes/sec]\n", (double)g_intf_stat.totalSendBytes / (real_intvl_time * 800));
    printf("Requests per second:      %ld[#/sec](mean)\n", total_requests / real_intvl_time);
    if (g_interval == 0) printf("Requests per second:      %ld[#/sec](adaptive)\n", final_speed);
    printf("Request size(Avg):        %ld\n", (size_t)(g_intf_stat.totalSendBytes / total_decimal));
    printf("Response size(Avg):       %ld\n", (size_t)(g_intf_stat.totalRecvBytes / total_decimal));
    printf("Latency time(Avg):        %2.2f[ms]\n", g_intf_stat.totalTime / total_decimal);
    printf("Latency time(P90):        %2.2f[ms]\n", g_intf_stat.p90Time);
    printf("Latency time(P99):        %2.2f[ms]\n", g_intf_stat.p99Time);
    printf("Latency time(P999):       %2.2f[ms]\n", g_intf_stat.p999Time);
    printf("\n\n");

    printf("Percentage of the requests served within a certain time\n");
    printCost(g_intf_stat);
}

// 函数入口
int main(int argc, char *argv[])
{
    // 环境初始化
    vector<int> pid_list;
    if (initialize(argc, argv) != 0)
    {
        return 0;
    }

    // 注册信号
    signal(SIGINT, procSignal);
    signal(SIGKILL, procSignal);
    signal(SIGUSR1, procSignal);
    signal(SIGTERM, procSignal);

    // 创建子进程进行压测
    g_begin_time = TBNOWMS;
    for (size_t ip = 0; ip < g_run_cores; ip++)
    {
        int pid = fork();
        if (pid == 0)
        {
            signal(SIGPIPE, SIG_IGN);
            return run(ip, argc, argv);
        }
        pid_list.push_back(pid);
    }

    // 主进程外显
    printPeriod(0);
    int64_t cur_time = TBNOWMS;
    int test_intvl = LICODE_GETINT("-I", 3600); // 默认1小时
    int view_intvl = LICODE_GETINT("-i", 5);
    while (test_intvl-- > 0 && g_run_flag)
    {
        if (abs(TBNOWMS - cur_time) > view_intvl * 1000)
        {
            cur_time = TBNOWMS;
            printPeriod(view_intvl);
        }
        sleep(1);
    }

    // 先杀死进程，然后打印统计
    for (size_t ii = 0; ii < pid_list.size(); ii++)
    {
        kill(pid_list[ii], SIGUSR1);
    }

    int status;
    while (wait(&status) > 0);
    printFinal((TBNOWMS - g_begin_time) / 1000);
    return 0;
}
