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
        // DECLARE_DYNCREATE(Protocol)
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
         * @brief  协议是否支持有序的
         *
         * @return 0成功, 其他失败
         */
        virtual int isSupportSeq() { return false; }

        /**
         * 验证数据包的完整性
         *
         * @param data 数据包指针
         * @param len  网络数据包长度
         *
         * @return 参考PACKET_XXXX;
         */
        virtual int input(const char *buf, size_t len) = 0;

        /**
         * 解析/编码协议, 获取网络数据包的实例
         *
         * @param req  业务数据实例
         * @param len  网络数据包长度
         * @param uniqId  全局唯一ID
         *
         * @return 0成功, 其他失败
         */
        virtual int encode(char *buf, int& len, int& uniqId) = 0;
        virtual int decode(const char *buf, int len, int& uniqId) = 0;
    };
};
#endif
