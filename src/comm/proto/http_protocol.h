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
#ifndef _HTTP_PROTOCOL_H_
#define _HTTP_PROTOCOL_H_

#include "protocol.h"

namespace bm
{
    class httpProtocol : public Protocol
    {
        DECLARE_DYNCREATE(httpProtocol)
    public:
        httpProtocol() {}
        virtual ~httpProtocol() {}

        /**
         * @brief  静态初始化函数
         *
         * @param argc 参数个数
         * @param argv 参数内容
         *
         * @return 0成功, 其他失败
         */
        virtual int initialize(int argc, char** argv);

        /**
         * @brief  初始化函数
         *
         * @param params  参数
         *
         * @return 0成功, 其他失败
         */
        virtual int initialize(const vector<string>& params);

        /**
         * @brief  判断收包是否完整
         *
         * @param buf  数据包指针
         * @param len  网络数据包长度
         *
         * @return int =0报文不完整; >0:实际报文长度; <0:出错
         */
        virtual int input(const char *buf, size_t len);

        /**
         * @brief  HTTP协议打包
         *
         * @param buf  数据包指针
         * @param len  数据包长度
         * @param uniqId  全局唯一ID
         *
         * @return 0成功, <0: 失败 >0: 需要的buffer长度
         */
        virtual int encode(char *buf, int& len, int& uniqId);

        /**
         * @brief  HTTP协议解包
         *
         * @param buf  数据包指针
         * @param len  数据包长度
         * @param uniqId  全局唯一ID
         *
         * @return 0成功, 其他失败
         */
        virtual int decode(const char *buf, int len, int& uniqId);
    private:
        /**
         * @brief  HTTP协议填充
         *
         * @param url       目标URL
         * @param header    http header
         * @param cookie    http cookie
         * @param body      POST内容    
         *
         * @return 0成功, 其他失败
         */
        virtual int fill_http_body(const string& url, const string& header, const string& cookie, const string& body);

    private:
        string  _reqBuff;
    };
};
#endif
