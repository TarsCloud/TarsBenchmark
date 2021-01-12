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
    void Transport::initialize(Monitor *monitor, const string &proto, int argc, char **argv)
    {
        _proto = _factory.get(proto);
        _proto->initialize(argc, argv);
        _monitor = monitor;
    }

    void Transport::initialize(Monitor *monitor, Protocol *protocol)
    {
        _proto = protocol;
        _monitor = monitor;
    }

    int Transport::handleWrite()
    {
        if (checkSocket() < 0)
        {
            return BM_SOCK_INVALID;
        }

        // 发送上次剩余数据
        while (_send_buff.size() > 0)
        {
            int snd_len = this->send(_send_buff.c_str(), _send_buff.size());
            if (snd_len < 0)
            {
                _monitor->report(BM_SOCK_SEND_ERROR);
                return BM_SOCK_SEND_ERROR;
            }
            else if (snd_len == 0 && errno == EAGAIN)
            {
                break; // 缓冲区数据太多，先处理一下回包
            }

            _send_buff.erase(_send_buff.begin(), _send_buff.begin() + snd_len);
        }
        return BM_SUCC;
    }

    int Transport::handleRead()
    {
        if (checkSocket() < 0)
        {
            _monitor->report(BM_SOCK_INVALID);
            return BM_SOCK_INVALID;
        }

        size_t rcv_len = MAX_RECVBUF_SIZE;
        char buff[MAX_RECVBUF_SIZE] = {0};
        while (this->recv(buff, rcv_len) == BM_SUCC && rcv_len > 0)
        {
            _monitor->reportRecv(TBNOWMS, (int)rcv_len);
            _recv_buff.append(buff, rcv_len);
            rcv_len = MAX_RECVBUF_SIZE;
        }

        handleProcess();
        return BM_SUCC;
    }

    int Transport::handleError()
    {
        close();
        checkSocket();
        _monitor->report(BM_SOCK_ERROR);
        return BM_SUCC;
    }

    int Transport::handleProcess()
    {
        int rcv_len = 0;
        int64_t cur_time = TBNOWMS;
        while ((rcv_len = _proto->input(_recv_buff.c_str(), _recv_buff.length())) > 0)
        {
            int seq_no = -1;
            int ret = _proto->decode(_recv_buff.c_str(), rcv_len, seq_no);
            auto it = _proto->isSupportSeq() ? _send_queue.find(seq_no) : _send_queue.begin();
            if (it != _send_queue.end())
            {
                _monitor->report(ret, (cur_time - it->second));
                _send_queue.erase(it);
            }

            _recv_buff.erase(_recv_buff.begin(), _recv_buff.begin() + rcv_len);
        }

        checkTimeOut(cur_time);
        return BM_SUCC;
    }

    bool Transport::checkTimeOut(int64_t cur_time)
    {
        if (checkSocket() < 0)
        {
            return false;
        }

        if (_conn_state == eConnecting && _con_timeout < cur_time)
        {
            this->close();
            return false;
        }

        for (auto it = _send_queue.begin(); it != _send_queue.end();)
        {
            if ((cur_time - it->second) > _ep.getTimeout())
            {
                _send_queue.erase(it++);
                _monitor->report(BM_SOCK_RECV_TIMEOUT, _ep.getTimeout());
            }
            else
            {
                ++it;
            }
        }
        return true;
    }

    int Transport::trySend(int uniq_no)
    {
        if (this->checkSocket() < 0)
        {
            _monitor->report(BM_SOCK_INVALID);
            return BM_SOCK_INVALID;
        }

        if (!this->checkConnect())
        {
            _monitor->report(BM_SOCK_CONN_ERROR);
            return BM_SOCK_CONN_ERROR;
        }

        int64_t cur_time = TBNOWMS;
        if (_send_queue.size() && !_proto->isSupportSeq())
        {
            return BM_SEQUENCE;
        }

        int seq_no = uniq_no;
        int buflen = MAX_SENDBUF_SIZE;
        static __thread char buf[MAX_SENDBUF_SIZE];
        int retCode = _proto->encode(buf, buflen, seq_no);
        if (retCode != 0)
        {
            _monitor->report(retCode);
            return BM_PACKET_ENCODE;
        }

        // 启动一次发送
        _monitor->reportSend(cur_time, buflen);
        _send_buff.append(buf, buflen);
        _send_queue[seq_no] = cur_time;
        handleWrite();
        return 0;
    }

    void Transport::close()
    {
        TC_ClientSocket::close();

        _send_queue.clear();
        _send_buff.clear();
        _recv_buff.clear();
        _conn_state = eUnconnected;
        _loop->del(getfd(), (uint64_t)this, EPOLLIN | EPOLLOUT);
    }

    void Transport::handle(TC_Epoller *loop, int wait)
    {
        int num = loop->wait(wait);
        for (int i = 0; i < num; ++i)
        {
            const epoll_event &ev = loop->get(i);
            Transport *conn = (Transport *)loop->getU64(ev);
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
        if (!getSocket()->isValid())
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
            _conn_state = eConnected;
            getSocket()->setblock(false);
            _loop->add(getfd(), (uint64_t)this, EPOLLIN | EPOLLOUT);
        }
        return BM_SUCC;
    }

    bool TCPTransport::checkConnect()
    {
        if (_conn_state == eConnected)
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
            _conn_state = eConnected;
            return true;
        }
        return false;
    }

    int TCPTransport::checkSocket()
    {
        if (!getSocket()->isValid())
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
                _conn_state = eConnecting;
                _con_timeout = _ep.getTimeout() + TBNOWMS;
                _loop->add(getfd(), (uint64_t)this, EPOLLOUT | EPOLLIN);
            }
            catch (TC_Exception &ex)
            {
                getSocket()->close();
                return BM_SOCK_ERROR;
            }
        }
        return BM_SUCC;
    }
}; // namespace bm
