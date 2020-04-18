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
#ifndef _COMMDEFS_H_
#define _COMMDEFS_H_

#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "licote.h"
#include "util/tc_common.h"
#include "util/tc_epoller.h"
#include "util/tc_clientsocket.h"
#include "util/tc_timeprovider.h"

using namespace std;
using namespace tars;

enum ErrorCode
{
    BM_SUCC = 0,
    BM_UNEXPECT           = 9000,
    BM_SEQUENCE           = 9001,
    BM_INIT_PARAM         = -101,

    BM_PACKET_ERROR       = -1000,
    BM_PACKET_ENCODE      = -1001,
    BM_PACKET_DECODE      = -1002,
    BM_PACKET_PARAM       = -1003,
    BM_PACKET_OVERFLOW    = -1004,

    BM_SOCK_ERROR         = -2000,
    BM_SOCK_INVALID       = -2001,
    BM_SOCK_NCONNECTED    = -2002,
    BM_SOCK_CONN_ERROR    = -2003,
    BM_SOCK_CONN_TIMEOUT  = -2004,
    BM_SOCK_SEND_ERROR    = -2005,
    BM_SOCK_RECV_ERROR    = -2006,
    BM_SOCK_RECV_TIMEOUT  = -2007,

    BM_SHM_ERR_GET        = -3000,
    BM_SHM_ERR_ATTACH     = -3001,
    BM_SHM_ERR_INIT       = -3002,
    BM_SHM_ERR_CLEAR      = -3003,
};

enum ConnectStatus
{
    eUnconnected,
    eConnecting,
    eConnected,
};

enum ConnectBuffer
{
    MAX_RECVBUF_SIZE = 64*1024,
    MAX_SENDBUF_SIZE = 4*1024*1024,
};

enum
{
    STA_FLAG = 0x5,
    END_FLAG = 0xa,
    MAX_STEP_COST = 10,
};

/**
 * @brief  string转换map
 *
 * @param ss  目标string
 *
 * @return map<int, int>
 */
inline map<int, int> str2map(const string &ss)
{
    int flag = 0;
    map<int, int> mm;
    string name, val, buff;
    char ch1 = ':', ch2 = ',';
    for (string::size_type pos = 0; pos < ss.length(); pos++)
    {
        //中间分隔符,前面读入是name
        if (ss[pos] == ch1)
        {
            flag = 1;
            name = buff;
            buff = "";
        }
        else if (ss[pos] == ch2 || (pos + 1) == ss.length())  //结束符,读入的是值
        {
            val  = buff;
            buff = "";
            if ((pos + 1) == ss.length())
            {
                val += ss[pos];
            }

            if (name.length() > 0 && flag)
            {
                mm[atoi(name.c_str())] = atoi(val.c_str());
                flag = 0;
            }
        }
        else
        {
            buff += ss[pos];
        }
    }
    return mm;
}

/**
 * @brief  map转换string
 *
 * @param mm 目标map
 *
 * @return string
 */
inline string map2str(const map<int, int>& mm)
{
    ostringstream os;
    for (map<int, int>::const_iterator itm = mm.begin(); itm != mm.end(); itm++)
    {
        os << itm->first << ":" << itm->second << ",";
    }
    return tars::TC_Common::trim(os.str(), ",");
}

/**
 * @brief 获取当前CPU数量
 *
 * @return int
 */
inline int64_t getProcNum(void)
{
    static int np = 0;
    if (np == 0)
    {
        np = sysconf(_SC_NPROCESSORS_ONLN);
    }
    return np;
}

#define MAX_FD  50001
#define TBNOWMS TC_Common::now2ms()
#define LICODE_GETINT(x, v) (licote_option_exist(x) ? TC_Common::strto<int>(licote_option_get(x)) : v)
#define LICODE_GETSTR(x, v) (licote_option_exist(x) ? TC_Common::trim(licote_option_get(x)) : v)

#define CAS(ptr, old, new)({ char ret; __asm__ __volatile__("lock; cmpxchgl %2,%0; setz %1": "+m"(*ptr), "=q"(ret): "r"(new),"a"(old): "memory"); ret;})
#define WMB() __asm__ __volatile__("sfence":::"memory")
#define RMB() __asm__ __volatile__("lfence":::"memory")

#endif // _COMMDEFS_H_

