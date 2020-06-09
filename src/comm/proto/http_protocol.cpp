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
#include "commdefs.h"
#include "http_protocol.h"
#include "util/tc_http.h"

using namespace tars;

namespace bm
{
    IMPLEMENT_DYNCREATE(httpProtocol, httpProtocol)

    int httpProtocol::initialize(int argc, char** argv)
    {
        // 支持命令
        licote_option_add("-H", "o",  "add header content");
        licote_option_add("-C", "o",  "set cookie content");
        licote_option_add("-F", "o",  "post file");
        licote_option_add("-u", NULL, "target url");
        licote_option_init(argc, argv);

        // 处理URL
        TC_URL cUrl;
        if (!cUrl.parseURL(LICODE_GETSTR("-u", "")))
        {
            licote_option_help("参数格式不正确: 错误的目的URL\n");
        }

        string post_body;
        string file_name = LICODE_GETSTR("-F", "");
        if (!file_name.empty())
        {
            ifstream ifs(file_name.c_str());
            post_body = string(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
        }

        return fill_http_body(cUrl.getURL(), LICODE_GETSTR("-H", ""), LICODE_GETSTR("-C", ""), post_body);
    }

    int httpProtocol::initialize(const vector<string>& params)
    {
        if (params.size() != 4)
        {
            return BM_INIT_PARAM;
        }

        // 处理URL
        TC_URL cUrl;
        if (!cUrl.parseURL(params[0]))
        {
            return BM_ERROR_URL;
        }

        return fill_http_body(cUrl.getURL(), params[1], params[2], params[3]);
    }

    int httpProtocol::fill_http_body(const string& url, const string& header, const string& cookie, const string& body)
    {
        TC_HttpRequest http;
        http.setConnection("Keep-Alive");

        // 处理Header
        if (!header.empty())
        {
            vector<string> vh = TC_Common::sepstr<string>(header, ";");
            for (size_t ii = 0; ii < vh.size(); ii++)
            {
                vector<string> kvh = TC_Common::sepstr<string>(vh[ii], ":");
                if (kvh.size() == 2)
                {
                    http.setHeader(kvh[0], kvh[1]);
                }
            }
        }

        // 处理Cookie
        if (!cookie.empty())
        {
            http.setCookie(cookie);
        }

        // 处理POST/GET
        if (!body.empty())
        {
            http.setPostRequest(url, body);
        }
        else
        {
            http.setGetRequest(url);
        }

        _reqBuff = http.encode();
        return 0;
    }

    int httpProtocol::encode(char *buf, int& len, int& uniqId)
    {
        ostringstream oss;
        try
        {
            if ((size_t)len < _reqBuff.length())
            {
                return _reqBuff.length();
            }

            uniqId = 1;
            len    = _reqBuff.length();
            memcpy(buf, _reqBuff.c_str(), len);
            return 0;
        }
        catch (exception& e)
        {
            oss << "std exception:" << e.what() << endl;
        }
        catch (...)
        {
            oss << "unknow exception" << endl;
        }
#ifdef DEBUG_SWITCH
        cerr << __FILE__ << ":" << __LINE__ << "|" << oss.str() << endl;
#endif
        return BM_PACKET_ENCODE;
    }

    int httpProtocol::decode(const char *buf, int len, int& uniqId)
    {
        ostringstream oss;
        try
        {
            TC_HttpResponse httpRsp;
            if (!httpRsp.decode(buf, len))
            {
                return BM_PACKET_DECODE;
            }

            if (httpRsp.getStatus() == 0)
            {
                uniqId = -1;
                return 0;
            }

            uniqId = 1;
            return httpRsp.getStatus() == 200 ? 0 : httpRsp.getStatus();
        }
        catch (exception& e)
        {
            oss << "std::exception: " << e.what();
        }
        catch (...)
        {
            oss << "unknow exception";
        }
#ifdef DEBUG_SWITCH
        cerr << __FILE__ << ":" << __LINE__ << "|" << oss.str() << endl;
#endif
        return BM_PACKET_DECODE;
    }

    int httpProtocol::input(const char *buf, size_t len)
    {
        char* posBody  = const_cast<char*>(strstr(buf, "\r\n\r\n"));
        if (posBody == NULL)
        {
            return 0;
        }

        // 解析头部
        posBody += 4;
        size_t lenHead = (size_t)(posBody - buf);
        if (lenHead > len)
        {
            return BM_PACKET_ERROR;
        }

        TC_HttpResponse httpRsp;
        httpRsp.parseResponseHeaderString(buf, posBody);
        if (httpRsp.getStatus() == 204 || httpRsp.getStatus() == 301 || httpRsp.getStatus() == 302)
        {
            // 直接返回
            return lenHead;
        }

        size_t lenContent = 0;
        if (!httpRsp.getHeader("Content-Length").empty())
        {
            lenContent = httpRsp.getContentLength();
        }

        while (httpRsp.getHeader("Transfer-Encoding") == "chunked")
        {
            char* posChunk  = strstr(posBody, "\r\n");
            if (posChunk == NULL)
            {
                return 0;
            }

            //查找当前chunk的大小
            size_t lenBody = (int)(posChunk - buf + 2);
            string sChunkSize = string(posBody, (size_t)(posChunk - posBody));
            int lenChunk  = strtol(sChunkSize.c_str(), NULL, 16);
            if (lenChunk <= 0)
            {
                return lenBody <= len ? lenBody : BM_PACKET_ERROR;
            }

            if (lenBody > len || (lenBody + lenChunk + 2) > len)
            {
                return BM_PACKET_ERROR;
            }
            posBody = posChunk + 2;
        }
        return lenHead + lenContent;
    }
};
