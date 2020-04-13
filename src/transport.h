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
#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#include "monitor.h"
#include "commdefs.h"
#include "protocol.h"
#include "proto_factory.h"

namespace bm
{
    class Transport : TC_ClientSocket
    {
    public:
        Transport(const TC_Endpoint& ep, TC_Epoller* loop) : _ep(ep), _loop(loop)
        {
            this->close();
            this->init(ep.getHost(), ep.getPort(), ep.getTimeout());
        }

        virtual ~Transport()
        {
            _factory.destroyObject();
            close();
        }

        /**
         * @brief  初始化函数
         * @param proto     协议名称
         * @param argc      参数个数
         * @param argv      参数内容
         *
         */
        virtual void initialize(const string& proto, int argc, char** argv)
        {
            _proto = _factory.get(proto, argc, argv);
        }

        /**
         * @brief  关闭资源
         *
         */
        virtual void close();

        /**
         * @brief  事件处理
         *
         * @return int
         */
        virtual int handleRead();
        virtual int handleError();
        virtual int handleWrite();
        virtual int handleProcess();

        /**
         * @brief  检查Socket操作
         *
         * @return int
         */
        virtual int checkSocket() = 0;

        /**
         * @brief  检查建链是否成功
         *
         * @return bool
         */
        virtual bool checkConnect() { return true; }

        /**
         * @brief  检查是否已经超时
         *
         * @return bool
         */
        virtual bool checkTimeOut(int64_t tCurTime);

        /**
         * @brief  获取句柄/socket
         *
         * @return int
         */
        virtual TC_Socket *getSocket() { return TC_ClientSocket::getSocket(); }
        virtual int getfd() { return this->getSocket()->getfd(); }

        /**
         * @brief  尝试发送
         * @param uniqId 业务数据实例
         *
         * @return int
         */
        int trySend(int uniqId);

        /**
         * @brief  处理epoll时间
         *
         */
        static void handle(TC_Epoller* loop, int time);
    protected:
        TC_Endpoint             _ep;
        TC_Epoller*             _loop;
        Protocol*               _proto;
        ProtoFactory            _factory;

        string                  _sendBuffer;
        string                  _recvBuffer;
        int64_t                 _conTimeOut;
        ConnectStatus           _connStatus;
        unordered_map<int, int64_t> _sendQueue;
    };

    class TCPTransport : public Transport
    {
    public:
        TCPTransport(const TC_Endpoint& ep, TC_Epoller* loop): Transport(ep, loop)
        {

        }

        /**
         * @brief  检查Socket操作
         *
         * @return bool
         */

        virtual int checkSocket();
        virtual bool checkConnect();

        /**
         * 发送函数
         *
         * @param buf  数据指针
         * @param len  数据长度
         * @param flag 标志位
         *
         * @return 0成功, 其他失败
         */
        virtual int send(const char *buf, size_t len)
        {
            if (getfd() == -1) return BM_SOCK_INVALID;
            if (eConnected != _connStatus) return BM_SOCK_CONN_ERROR;

            int ret = getSocket()->send(buf, len);
            if (ret < 0 && errno != EAGAIN)
            {
                this->close();
                return BM_SOCK_SEND_ERROR;
            }
            return ret < 0 ? 0 : ret;
        }

        /**
         * 接收处理函数
         *
         * @param buf  数据指针
         * @param len  数据长度
         *
         * @return 0成功, 其他失败
         */
        int recv(char *buf, size_t& len)
        {
            if (getfd() == -1)
            {
                return BM_SOCK_INVALID;
            }

            int rcvLen = this->getSocket()->recv((void*)buf, len, 0);
            if (rcvLen < 0 && errno != EAGAIN)
            {
                close();
                Monitor::getInstance()->report(BM_SOCK_RECV_ERROR);
                return BM_SOCK_RECV_ERROR;
            }

            len = rcvLen <= 0 ? 0 : rcvLen;
            return BM_SUCC;
        }
    };

    class UDPTransport : public Transport
    {
    public:
        UDPTransport(const TC_Endpoint& ep, TC_Epoller* loop) : Transport(ep, loop)
        {

        }

        /**
         * @brief  检查Socket操作
         *
         * @return bool
         */
        int checkSocket();

        /**
         * 发送函数
         *
         * @param buf  数据指针
         * @param len  数据长度
         *
         * @return 0成功, 其他失败
         */
        int send(const char *buf, size_t len)
        {
            if (getfd() < 0) return BM_SOCK_INVALID;

            int ret = this->getSocket()->sendto(buf, len, _ep.getHost(), _ep.getPort(), 0);
            if (ret < 0 && errno != EAGAIN)
            {
                close();
                return BM_SOCK_SEND_ERROR;
            }
            return ret <= 0 ? 0 : (int)len;
        }

        /**
         * 接收处理函数
         *
         * @param buf  数据指针
         * @param len  数据长度
         * @param flag 标志位
         *
         * @return 0成功, 其他失败
         */
        int recv(char *buf, size_t& len)
        {
            if (getfd() < 0) return BM_SOCK_INVALID;

            string tmpIp;
            uint16_t tmpPort;
            int rcvLen = this->getSocket()->recvfrom(buf, len, tmpIp, tmpPort);
            if (rcvLen < 0 && errno != EAGAIN)
            {
                this->close();
                Monitor::getInstance()->report(BM_SOCK_RECV_ERROR);
                return BM_SOCK_CONN_ERROR;
            }

            len = rcvLen <= 0 ? 0 : rcvLen;
            return BM_SUCC;
        }
    };
};
#endif
