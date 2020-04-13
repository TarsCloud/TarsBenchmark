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
#include <transport.h>
#include <netinet/tcp.h>
#include "util/tc_common.h"
using namespace tars;

#define CONNECT_TIMEOUT_MS 3000
namespace bm
{
    int Transport::handleWrite()
    {
        if (checkSocket() < 0)
        {
            return BM_SOCK_INVALID;
        }

        // 发送上次剩余数据
        while(_sendBuffer.size() > 0)
        {
            int sndLen = this->send(_sendBuffer.c_str(), _sendBuffer.size());
            if (sndLen < 0)
            {
                Monitor::getInstance()->report(BM_SOCK_SEND_ERROR);
                return BM_SOCK_SEND_ERROR;
            }
            else if(sndLen == 0 && errno == EAGAIN)
            {
                break; // 缓冲区数据太多，先处理一下回包
            }

            _sendBuffer.erase(_sendBuffer.begin(), _sendBuffer.begin() + sndLen);
        }
        return BM_SUCC;
    }

    int Transport::handleRead()
    {
        if (checkSocket() < 0)
        {
            Monitor::getInstance()->report(BM_SOCK_INVALID);
            return BM_SOCK_INVALID;
        }

        size_t rcvLen = MAX_RECVBUF_SIZE;
        char buff[MAX_RECVBUF_SIZE] = {0};
        while (this->recv(buff, rcvLen) == BM_SUCC && rcvLen > 0)
        {
            _recvBuffer.append(buff, rcvLen);
            Monitor::getInstance()->reportRecv(TBNOWMS, (int)rcvLen);
        }

        handleProcess();
        return BM_SUCC;
    }

    int Transport::handleError()
    {
        close();
        checkSocket();
        Monitor::getInstance()->report(BM_SOCK_ERROR);
        return BM_SUCC;
    }

    int Transport::handleProcess()
    {
        int iRecvLen = 0;
        int64_t tCurTime = TBNOWMS;
        while ((iRecvLen = _proto->input(_recvBuffer.c_str(), _recvBuffer.length())) > 0)
        {
            int reqIdx = -1;
            int ret = _proto->decode(_recvBuffer.c_str(), iRecvLen, reqIdx);
            auto it = _proto->isSupportSeq() ? _sendQueue.find(reqIdx) : _sendQueue.begin();
            if (it != _sendQueue.end())
            {
                Monitor::getInstance()->report(ret, (tCurTime - it->second));
                _sendQueue.erase(it);
            }

            _recvBuffer.erase(_recvBuffer.begin(), _recvBuffer.begin() + iRecvLen);
        }

        checkTimeOut(tCurTime);
        return BM_SUCC;
    }

    bool Transport::checkTimeOut(int64_t tCurTime)
    {
        if (checkSocket() < 0)
        {
            return false;
        }

        if (_connStatus == eConnecting && _conTimeOut < tCurTime)
        {
            this->close();
            return false;
        }

        for(auto it = _sendQueue.begin(); it != _sendQueue.end(); )
        {
            if ((tCurTime - it->second) > _ep.getTimeout())
            {
                _sendQueue.erase(it++);
                Monitor::getInstance()->report(BM_SOCK_RECV_TIMEOUT, _ep.getTimeout());
            }
            else
            {
                ++it;
            }
        }
        return true;
    }

    int Transport::trySend(int uniqId)
    {
        if (this->checkSocket() < 0)
        {
            Monitor::getInstance()->report(BM_SOCK_INVALID);
            return BM_SOCK_INVALID;
        }

        if (!this->checkConnect())
        {
            Monitor::getInstance()->report(BM_SOCK_CONN_ERROR);
            return BM_SOCK_CONN_ERROR;
        }

        int64_t tCurTime = TBNOWMS;
        if (_sendQueue.size() && !_proto->isSupportSeq())
        {
            return BM_SEQUENCE;
        }

        int reqIdx = uniqId;
        int bufLen = MAX_SENDBUF_SIZE;
        static char buf[MAX_SENDBUF_SIZE];
        int iRet = _proto->encode(buf, bufLen, reqIdx);
        if (iRet != 0)
        {
            Monitor::getInstance()->report(iRet);
            return BM_PACKET_ENCODE;
        }

        // 启动一次发送
        Monitor::getInstance()->reportSend(tCurTime, bufLen);
        _sendBuffer.append(buf, bufLen);
        _sendQueue[reqIdx] = tCurTime;
        handleWrite();
        return 0;
    }

    void Transport::close()
    {
        TC_ClientSocket::close();

        _sendQueue.clear();
        _sendBuffer.clear();
        _recvBuffer.clear();
        _connStatus = eUnconnected;
        _loop->del(getfd(), (uint64_t)this, EPOLLIN|EPOLLOUT);
    }

    void Transport::handle(TC_Epoller* loop, int time)
    {
        int num = loop->wait(time);
        for (int i = 0; i < num; ++i)
        {
            const epoll_event& ev = loop->get(i);
            Transport *conn = (Transport*)loop->getU64(ev);
            if (conn != NULL)
            {
                // 强制读一下
                conn->handleRead();
                if (loop->writeEvent(ev))
                {
                    conn->handleWrite();
                }

                // 出错执行
                if (loop->errorEvent(ev))
                {
                    conn->handleError();
                }
            }
        }
    }

    int UDPTransport::checkSocket()
    {
        if(!getSocket()->isValid())
        {
            try
            {
    #if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
                getSocket()->createSocket(SOCK_DGRAM, AF_INET);
    #else
                getSocket()->createSocket(SOCK_DGRAM, AF_INET);
    #endif
            }
            catch (TC_Exception &ex)
            {
                getSocket()->close();
                return BM_SOCK_ERROR;
            }

            //设置非阻塞模式
            _connStatus = eConnected;
            getSocket()->setblock(false);
            _loop->add(getfd(), (uint64_t)this, EPOLLIN|EPOLLOUT);
        }
        return BM_SUCC;
    }

    bool TCPTransport::checkConnect()
    {
        if (_connStatus == eConnected)
        {
            return true;
        }
#if TARGET_PLATFORM_IOS
        #include <netinet/tcp_fsm.h>
        struct tcp_connection_info info;
        int len = sizeof(info);
        int ret = ::getsockopt(getfd(), IPPROTO_TCP, TCP_CONNECTION_INFO, &info, (socklen_t *)&len);
        if (ret == 0 && info.tcpi_state == TCPS_ESTABLISHED)
#else
        struct tcp_info info;
        int len = sizeof(info);
        int ret = ::getsockopt(getfd(), IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
        if (ret == 0 && info.tcpi_state == TCP_ESTABLISHED)
#endif
        {
            _connStatus = eConnected;
            return true;
        }
        return false;
    }

    int TCPTransport::checkSocket()
    {
        if(!getSocket()->isValid())
        {
            try
            {
    #if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
                getSocket()->createSocket(SOCK_STREAM, AF_INET);
    #else
                getSocket()->createSocket(SOCK_STREAM, AF_INET);
    #endif
                getSocket()->setblock(false);
                getSocket()->setNoCloseWait();
                int ret = getSocket()->connectNoThrow(_ep.getHost(), _ep.getPort());
                if (ret < 0 && !TC_Socket::isInProgress())
                {
                    getSocket()->close();
                    return BM_SOCK_CONN_ERROR;
                }

                //设置非阻塞模式
                _connStatus = eConnecting;
                _conTimeOut = _ep.getTimeout() + TBNOWMS;
                _loop->add(getfd(), (uint64_t)this, EPOLLOUT | EPOLLIN);
            }
            catch(TC_Exception &ex)
            {
                getSocket()->close();
                return BM_SOCK_ERROR;
            }
        }
        return BM_SUCC;
    }
};
