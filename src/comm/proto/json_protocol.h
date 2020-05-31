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
#ifndef _TARS_JSON_PROTOCOL_H_
#define _TARS_JSON_PROTOCOL_H_

#include <string>
#include <vector>
#include <tup/Tars.h>
#include <tup/TarsJson.h>
#include <tup/RequestF.h>
#include "protocol.h"

using namespace std;
using namespace tars;

namespace bm
{
    struct JsonField
    {
    public:
        int     tag;                // tag
        bool    usigned;            // 是否非符号型
        string  type;               // 类型
        string  name;               // 变量名
        string  defval;             // 默认值
        vector<JsonField> child;    // 子对象
    };

    inline bool compField(const JsonField& l, const JsonField& r)
    {
        return (l.tag < r.tag);
    }

    class jsonProtocol : public Protocol
    {
        DECLARE_DYNCREATE(jsonProtocol)
    public:
        jsonProtocol() { _random_flag = false; }
        virtual ~jsonProtocol() {}
        /**
         * @brief  协议初始化函数
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
         * @brief  协议是否支持有序的
         *
         * @return true: 支持, false: 不支持
         */
        virtual int isSupportSeq() { return true; }

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
         * @brief  TARS服务协议打包
         *
         * @param buf  数据包指针
         * @param len  数据包长度
         * @param uniqId  全局唯一ID
         *
         * @return 0成功, 其他失败
         */
        virtual int encode(char *buf, int& len, int& uniqId);
        virtual int decode(const char *buf, int len, int& uniqId);

        /**
         * @brief  TARS协议编码
         *
         * @param os        缓存区
         * @param f         成员类型
         * @param v         Json值
         *
         * @return 0成功, 其他失败
         */
        int encode(TarsOutputStream<BufferWriter> &os, JsonField& f, JsonValuePtr v);

        /**
         * @brief  TARS协议解码
         *
         * @param is        缓存区
         * @param f         成员类型
         *
         * @return JsonValuePtr
         */
        JsonValuePtr decode(TarsInputStream<BufferReader> &is, JsonField& f);

        /**
         * @brief  解析成员变量
         *
         * @param ptr    对象指针
         * @param field  数据编号
         * @param elem_num  预期的成员数量
         *
         * @return Field结构
         */
        void parseField(JsonValueObjPtr ptr, vector<JsonField>& field, int elem_num = -1);
        
    private:
        /**
         * @brief  解析测试用例
         *
         * @param in_param  用例参数
         * @param in_value  用例内容
         *
         * @return 0成功, 其他失败
         */
        int parseCase(const string& in_param, const string& in_value);
    public:
        int                     _timeout;       // 超时时间
        string                  _servant;       // servant名称
        string                  _function;      // rpc函数
        JsonValueObjPtr         _para_value;    // 输入参数
        vector<JsonField>       _para_field;    // 参数配置
        TarsOutputStream<BufferWriter> _os;
    };
};
#endif
