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
#ifndef _PROTOCOL_INCLUDE_
#define _PROTOCOL_INCLUDE_

#include "util/tc_dyn_object.h"

using namespace tars;

namespace bm
{
    class Protocol : public TC_DYN_Object
    {
    public:
        virtual ~Protocol() {}

        /**
         * @brief  初始化函数
         *
         * @param argc  参数
         * @param argv  参数内容
         *
         * @return 0成功, 其他失败
         */
        virtual int initialize(int argc, char** argv) = 0;

        /**
         * @brief  初始化函数
         *
         * @param params  参数
         *
         * @return 0成功, 其他失败
         */
        virtual int initialize(const vector<string>& params) { return 0; }

        /**
         * @brief  协议是否支持有序的
         *
         * @return true: 支持; false: 不支持
         */
        virtual int isSupportSeq() { return false; }

        /**
         * @brief  判断收包是否完整
         *
         * @param buf  数据包指针
         * @param len  网络数据包长度
         *
         * @return int =0报文不完整; >0:实际报文长度; <0:出错
         */
        virtual int input(const char *buf, size_t len) = 0;

        /**
         * 解析/编码协议, 获取网络数据包的实例
         *
         * @param buf  数据包指针
         * @param len  网络数据包长度
         * @param uniqId  全局唯一ID
         *
         * @return 0成功, 其他失败
         */
        virtual int encode(char *buf, int& len, int& uniqId) = 0;
        virtual int decode(const char *buf, int len, int& uniqId) = 0;
    protected:
        /**
         * @brief  生成取随机内容
         *
         * @param rmin  最小值
         * @param rmax  最大值
         *
         * @return long 随机值
         */
        virtual long genRandomValue(const string& rmin, const string& rmax)
        {
            long max = TC_Common::strto<long>(rmax);
            long min = TC_Common::strto<long>(rmin);
            return (long)(rand() % (max - min + 1) + min);
        }

        /**
         * @brief  生成取随机内容
         *
         * @param v     string内容
         * @param is_int    是否整形数据
         *
         * @return string 随机值内容
         */
        virtual string genRandomValue(const string& v, bool is_int = true)
        {
            string::size_type l = v.find_first_of('[');
            string::size_type r = v.find_last_of(']');
            string::size_type m = v.find_first_of('-');
            string::size_type n = v.find_first_of(',');
            if (l != 0 || r != (v.size() - 1) || (m == string::npos && n == string::npos))
            {
                return v;
            }

            string nv = v.substr(l + 1, r - l);
            if (m != string::npos && is_int)
            {
                vector<string> vs = TC_Common::sepstr<string>(nv, "-");
                if (vs.size() == 2)
                {
                    _random_flag = true;
                    return TC_Common::tostr(genRandomValue(vs.at(0), vs.at(1)));
                }
                throw runtime_error("invalid randval(-)");
            }
            else if (n != string::npos)
            {
                vector<string> vs = TC_Common::sepstr<string>(nv, ",");
                if (vs.size() > 1)
                {
                    _random_flag = true;
                    return vs[(size_t)rand() % vs.size()];
                }
            }
            return nv;
        }
    protected:
        bool _random_flag;
    };
};
#endif
