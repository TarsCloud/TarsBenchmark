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

#include <unordered_map>
#include <sys/types.h>
#include <sys/shm.h>
#include <sstream>
#include <fstream>
#include <errno.h>

#include "licote.h"
#include <tup/Tars.h>
#include "util/tc_common.h"
#include "util/tc_epoller.h"
#include "util/tc_clientsocket.h"
#include "util/tc_timeprovider.h"

using namespace std;
using namespace tars;

enum ErrorCode
{
    BM_SUCC               = 0,
    BM_SEQUENCE           = 1,
    BM_EXCEPTION          = -1,
    BM_INIT_PARAM         = -101,
    BM_ERROR_URL          = -102,

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

    BM_SERVER_ERR_PARAM     = -10000,
    BM_ADMIN_ERR_NOTFIND    = -10001,   // 未找到此链接
    BM_ADMIN_ERR_RUNNING    = -10002,   // 服务未运行
    BM_ADMIN_ERR_STARTUP    = -10003,   // 启动失败
    BM_ADMIN_ERR_SHUTDOWN   = -10004,   // 关闭失败
    BM_ADMIN_ERR_ENCODE     = -10005,   // 编码失败
    BM_ADMIN_ERR_DECODE     = -10006,   // 编码失败
    BM_ADMIN_ERR_SOCKET     = -10007,   // 网络失败
    BM_ADMIN_ERR_TASK       = -10008,   // 服务不在运行中
    BM_ADMIN_ERR_PROTO      = -10009,   // 未找到此协议

    BM_NODE_ERR_RUNNING     = -20001,   // 正在运行
    BM_NODE_ERR_RESOURCE    = -20002,   // 资源不足
    BM_NODE_ERR_CASEMATCH   = -20003,   // 用例参数和内容不匹配
    BM_NODE_ERR_CONNECTION  = -20004,   // 链接数设置不合理(整数倍，且不要超过500倍)
    BM_NODE_ERR_ENDPOINT    = -20005,   // 目标服务器配置不正确
    BM_NODE_ERR_RPCCALL     = -20006,   // 目标服务器调用失败
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

const string PT_VOID        = "void";
const string PT_BOOLEAN     = "bool";
const string PT_BYTE        = "byte";
const string PT_SHORT       = "short";
const string PT_INT         = "int";
const string PT_LONG        = "long";
const string PT_FLOAT       = "float";
const string PT_DOUBLE      = "double";
const string PT_STRING      = "string";
const string PT_VECTOR      = "vector";
const string PT_MAP         = "map";
const string PT_STRUCT      = "struct";
const string PT_UNSIGNED    = "unsigned";

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
 * 通用模板方法: Tars对象T序列化
 *
 * @param t 类型T的对象
 * @return 字符串
 */
template<typename T> string tostring(const T& t)
{
    string s;
    TarsOutputStream<BufferWriter> osk;
    t.writeTo(osk);
    s.assign(osk.getBuffer(), osk.getLength());

    return s;
}

/**
 * 通用模板方法: Tars对象T结构化
 *
 * @param t 字符串
 * @return 类型T的对象
 */
template<typename T> T& toObj(const string& s, T& t)
{
    TarsInputStream<BufferReader> isk;
    isk.setBuffer(s.c_str(), s.length());
    t.readFrom(isk);

    return t;
}


/**
 * 通用模板方法: Tars对象日志化打印
 *
 * @param t 类型T的对象
 * @return string
 */
template<typename T> std::string logTars(const T& t, bool simple = true)
{
    ostringstream os;
    simple ? t.displaySimple(os) : t.display(os);
    return os.str();
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

//过程控制
#ifndef PROC_BEGIN
#define PROC_BEGIN do{
#endif

#ifndef PROC_END
#define PROC_END   }while(0);
#endif

#ifndef PROC_TRY_BEGIN       // 带try版本
#define PROC_TRY_BEGIN        do{             \
    try{
#endif
#ifndef PROC_TRY_END    // 带try版本
#define PROC_TRY_END(msg, ret, errcode, defcode)    }   \
    catch(TC_Exception& e) {                            \
        (msg) = string("tars exception:") + e.what();   \
        (ret) = (errcode);                              \
    }                                                   \
    catch(std::exception& e) {                          \
        (msg) = string("std exception:") + e.what();    \
        (ret) = (errcode);                              \
    }                                                   \
    catch(...) {                                        \
        (msg) = "unknown exception";                    \
        (ret) = (defcode);                              \
    }                                                   \
}while(0);
#endif
#ifndef PROC_EXIT
#define PROC_EXIT(ret, value) {(ret)=(value);break;};
#endif
#ifndef PROC_TRY_EXIT
#define PROC_TRY_EXIT(ret,retval,code,errcode,desc,errdesc) {   \
        (ret)=(retval);                                         \
        (code) = (errcode);                                     \
        (desc) = (errdesc);                                     \
        break;                                                  \
    };
#endif
#ifndef PROC_EQ_EXIT
#define PROC_EQ_EXIT(expr, eval, ret, value) {if( (expr) == (eval) ) { (ret) = (value); break; } };
#endif
#ifndef PROC_NE_EXIT
#define PROC_NE_EXIT(expr, eval, ret, value) {if( (expr) != (eval) ) { (ret) = (value); break; } };
#endif

#define MAX_FD  50001
#define TBNOWMS TC_Common::now2ms()
#define LICODE_GETINT(x, v) (licote_option_exist(x) ? TC_Common::strto<int>(licote_option_get(x)) : v)
#define LICODE_GETSTR(x, v) (licote_option_exist(x) ? TC_Common::trim(licote_option_get(x)) : v)

#define CAS(ptr, old, new)({ char ret; __asm__ __volatile__("lock; cmpxchgl %2,%0; setz %1": "+m"(*ptr), "=q"(ret): "r"(new),"a"(old): "memory"); ret;})
#define WMB() __asm__ __volatile__("sfence":::"memory")
#define RMB() __asm__ __volatile__("lfence":::"memory")

#endif // _COMMDEFS_H_

