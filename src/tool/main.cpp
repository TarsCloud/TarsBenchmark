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
LICOTE_OPTION_DECL("-c", 	NULL, 	"number of connections");
LICOTE_OPTION_DECL("-D", 	NULL, 	"target server address(ipv4)");
LICOTE_OPTION_DECL("-P", 	NULL, 	"target server port");
LICOTE_OPTION_DECL("-T", 	"o", 	"network protocol(tcp|udp)");
LICOTE_OPTION_DECL("-I", 	"o", 	"continue time(by second)");
LICOTE_OPTION_DECL("-i", 	"o", 	"view interval");
LICOTE_OPTION_DECL("-t", 	"o", 	"overtime time");
LICOTE_OPTION_DECL("-s", 	"o", 	"maximum tps");
LICOTE_OPTION_DECL("-n", 	"o", 	"maximum process");
LICOTE_OPTION_DECL("-p", 	"o", 	"server protocol(tars|http)");
LICOTE_OPTION_DECL("-h", 	"h", 	"help info");

LICOTE_SET_LANGUAGE(CN);
LICOTE_SET_VERSION("BSD");
LICOTE_SET_EXAMPLE("./tb -c 2000 -s 8000 -D 192.168.16.1 -P 10505 -p tars -i 10 -S tars.DemoServer.DemoObj -M test -C test.case;"
                   "./tb -c 2000 -s 8000 -t 6000 -D 192.168.31.1,192.168.32.1 -P 80 -u http://www.qq.com");
LICOTE_SET_DESCRIPTION("tars benchmark tool");
LICOTE_OPTION_END

/**
*  全局变量定义
*/
bool                gRunFlag;
size_t              gSpeed;
size_t              gConsNum;
size_t              gInterval;
size_t              gRunCores;
string              gProtoName;
size_t              gStartTime;
IntfStat            gStatInf;
vector<TC_Endpoint> gEps;


/**
*  初始化函数
*/
int initialize(int argc, char* argv[])
{
    try
    {
        // LICOTE初始化
        if (licote_option_init(argc, argv) != 0)
        {
            return BM_INIT_PARAM;
        }

        // 获取压测核数
        gRunCores = std::max(LICODE_GETINT("-n", getProcNum()), (int64_t)1);

        // EndPoint初始化
        TC_Endpoint::EType netType = LICODE_GETSTR("-T", "tcp") != "udp" ? TC_Endpoint::TCP : TC_Endpoint::UDP;
        TC_Endpoint ep("", LICODE_GETINT("-P", 0), LICODE_GETINT("-t", 3000), netType);
        vector<string> vd = TC_Common::sepstr<string>(LICODE_GETSTR("-D", ""), ",");
        for (size_t ii = 0; ii < vd.size() && !vd[ii].empty(); ii++)
        {
            ep.setHost(vd[ii]);
            gEps.push_back(ep);
        }

        if (ep.getTimeout() == 0 || ep.getPort() == 0 || gEps.size() == 0)
        {
            licote_option_help("参数格式不正确: 目标服务器配置错误\n");
        }

        // 压测参数初始化
        gSpeed    = (LICODE_GETINT("-s", 0) + gRunCores - 1) / gRunCores;
        gConsNum  = (LICODE_GETINT("-c", 0) + gRunCores - 1) / gRunCores;
        if (gConsNum == 0  || LICODE_GETINT("-i", 10) == 0)
        {
            licote_option_help("参数格式不正确: -i|-c 不能为0\n");
        }
        else if (gConsNum * gEps.size() > MAX_FD)
        {
            licote_option_help("参数格式不正确: 连接数超过系统限制\n");
        }

        ProtoFactory protoFactory;
        gProtoName = LICODE_GETSTR("-p", "http") + "Protocol";
        if (protoFactory.get(gProtoName) == NULL)
        {
            licote_option_help("参数格式不正确: 协议不存在\n");
        }

        protoFactory.get(gProtoName)->initialize(argc, argv);
        gInterval = gSpeed == 0 ? 0 : (1000000 * gConsNum) / gSpeed;
        memset(&gStatInf, 0, sizeof(IntfStat));
        Monitor::getInstance()->initialize();
        gRunFlag = true;
    }
    catch (exception& e)
    {
        licote_option_help("系统异常: %s\n", e.what());
    }
    return 0;
}

/**
*  压测主程序run
*/
int run(int seqno, int argc, char* argv[])
{
    // 事件初始化
    TC_Epoller loop;
    loop.create(MAX_FD);

    // 创建连接
    vector<Transport*> connections;
    for (size_t i = 0; i < gConsNum; i++)
    {
        for (size_t ii = 0; ii < gEps.size(); ii++)
        {
            Transport* conn = gEps[ii].isTcp() ? (Transport*)(new TCPTransport(gEps[ii], &loop))
                                               : (Transport*)(new UDPTransport(gEps[ii], &loop));
            conn->initialize(Monitor::getInstance(), gProtoName, argc, argv);
            connections.push_back(conn);
        }
    }

    int64_t req_no = seqno;
    int64_t next_send_time = 0;
    while (gRunFlag)
    {
        try
        {
            int64_t cur_time = TC_Common::now2us();
            if (cur_time >= next_send_time)
            {
                next_send_time = next_send_time + gInterval;
                for (size_t i = 0; i < connections.size(); i++)
                {
                    req_no += gRunCores;
                    connections[i]->trySend(req_no);
                    connections[i]->checkTimeOut(cur_time/1000);
                }
            }

            Transport::handle(&loop, 1);
            Monitor::getInstance()->syncStat(cur_time/1000);
        }
        catch (tars::TC_Exception& e)
        {
            cerr << "tars exception:" << e.what() << endl;
        }
    }

    // 析构链接前收发一次
    Transport::handle(&loop, 1000);
    Monitor::getInstance()->syncStat(0);
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
        gRunFlag = false;
    }
}

/**
*  输出耗时统计
*/
void printCost(const IntfStat& stat)
{
    double totol_count = 0.0;
    double meger_count = 0.0;
    static int arrCost[MAX_STEP_COST] = {0, 10, 30, 50, 100, 500, 3000, 5000, 99999, 0};
    for (size_t i = 0; i < MAX_STEP_COST && arrCost[i + 1] != 0; i++)
    {
        totol_count += stat.costTimes[i];
    }

    totol_count = max(1.0, totol_count);
    for (size_t i = 0; i < MAX_STEP_COST && arrCost[i + 1] != 0; i++)
    {
        meger_count += stat.costTimes[i];
        printf("[%5d - %5d ms] %7d\t%2.2f%%\t\t%2.2f%%\n", arrCost[i], arrCost[i+1], stat.costTimes[i],
            100 * (double)stat.costTimes[i] / totol_count, 100 * meger_count / totol_count);
    }
}

/**
*  输出压测周期结果
*/
void printPeriod(int intvl_time)
{
    IntfStat stat;
    map<int, int> ret_final;
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
            stat += stat_list[ii];
        }
        gStatInf += stat;
    }

    if (intvl_time > 0)
    {
        double total_decimal = std::max(1.00, (double)stat.totalCount);
        double fail_rate = min(stat.failCount, stat.totalCount) / total_decimal;
        printf("\n\n--------------------------------------------------------------------------------------------------------------------\n");
        printf("Time\t\t\tTotal\tSucc\tFail\tRate\tMax(ms)\tMin(ms)\tAvg(ms)\tP90(ms)\tP99(ms)\tP999(ms)\tTPS\n");
        printf("%s\t%-6d\t%-6d\t%-6d\t%0.2f%%\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t\t%d",
            TC_Common::now2str("%Y-%m-%d %H:%M:%S").c_str(),
            stat.totalCount, stat.succCount, stat.failCount, (1 - fail_rate) * 100,
            stat.maxTime, stat.minTime, stat.totalTime/total_decimal,
            stat.p90Time, stat.p99Time, stat.p999Time,
            stat.totalCount/intvl_time);

        printf("\n\n\nCall Result: [%s]\n", map2str(ret_final).c_str());
        printf("--------------------------------------------------------------------------------------------------------------------\n");
        printCost(stat);
    }
}

/**
*  输出最终压测结果
*/
void printFinal(int intvl_time)
{
    printPeriod(0);
    string sEndPoint = gEps[0].toString();
    for (size_t ii = 1; ii < gEps.size(); ii++)
    {
        sEndPoint.append("\n                        ").append(gEps[ii].toString());
    }

    double total_decimal = std::max(1.00, (double)gStatInf.totalCount);
    double fail_rate = min((double)gStatInf.failCount, (double)gStatInf.totalCount) / total_decimal;

    int real_intvl_time = intvl_time - LICODE_GETINT("-t", 3000) / 3000;
    printf("\n\n--------------------------------------------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("----------------------------------   Finish Pressure Test   --------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("Server Numbers:         %ld\n", gEps.size());
    printf("Server Endpoint:        %s\n",  sEndPoint.c_str());
    printf("Server Protocol:        %s\n\n\n", LICODE_GETSTR("-p", "").c_str());

    printf("Concurrency Procesor:     %ld\n", gRunCores);
    printf("Concurrency Connections:  %lu\n", gConsNum * gRunCores);
    printf("Connections per Procesor: %ld\n", gConsNum);
    printf("Success requests:         %d\n", gStatInf.succCount);
    printf("Success rate:             %.2f%%\n", (1 - fail_rate) * 100);
    printf("Failed requests:          %d\n", gStatInf.failCount);
    printf("Total requests:           %d\n", gStatInf.totalCount);
    printf("Total duration:           %d[sec]\n", real_intvl_time);
    printf("Transfer rate:            %.2f[Kbytes/sec]\n", (double)gStatInf.totalSendBytes/(real_intvl_time*800));
    printf("Requests per second:      %d[#/sec](mean)\n", gStatInf.totalCount/real_intvl_time);
    printf("Request size(Avg):        %ld\n", (size_t)gStatInf.totalSendBytes/gStatInf.totalCount);
    printf("Response size(Avg):       %ld\n", (size_t)gStatInf.totalRecvBytes/gStatInf.totalCount);
    printf("Latency time(Avg):        %2.2f[ms]\n", gStatInf.totalTime/total_decimal);
    printf("Latency time(P90):        %2.2f[ms]\n", gStatInf.p90Time);
    printf("Latency time(P99):        %2.2f[ms]\n", gStatInf.p99Time);
    printf("Latency time(P999):       %2.2f[ms]\n", gStatInf.p999Time);
    printf("\n\n");

    printf("Percentage of the requests served within a certain time\n");
    printCost(gStatInf);
}

// 函数入口
int main(int argc, char* argv[])
{
    // 环境初始化
    vector<int> pid_list;
    if (initialize(argc, argv) != 0)
    {
        return 0;
    }

    // 注册信号
    signal(SIGINT,  procSignal);
    signal(SIGKILL, procSignal);
    signal(SIGUSR1, procSignal);
    signal(SIGTERM, procSignal);

    // 创建子进程进行压测
    gStartTime = TBNOWMS;
    for (size_t ip = 0; ip < gRunCores; ip++)
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
    int test_intvl = LICODE_GETINT("-I", 3600);   // 默认1小时
    int view_intvl = LICODE_GETINT("-i", 5);
    while (test_intvl-- > 0 && gRunFlag)
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
    printFinal((TBNOWMS - gStartTime) / 1000);
    return 0;
}

