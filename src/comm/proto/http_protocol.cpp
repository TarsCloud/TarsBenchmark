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

    int httpProtocol::initialize(int argc, char **argv)
    {
        // 支持命令
        licote_option_add("-H", "o", "add header content");
        licote_option_add("-C", "o", "set cookie content");
        licote_option_add("-F", "o", "post file");
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

    int httpProtocol::initialize(const vector<string> &params)
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

    int httpProtocol::fill_http_body(const string &url, const string &header, const string &cookie, const string &body)
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

        _req_buff = http.encode();
        return 0;
    }

    int httpProtocol::encode(char *buf, int &len, int &uniq_no)
    {
        ostringstream oss;
        try
        {
            if ((size_t)len < _req_buff.length())
            {
                return _req_buff.length();
            }

            uniq_no = 1;
            len = _req_buff.length();
            memcpy(buf, _req_buff.c_str(), len);
            return 0;
        }
        catch (exception &e)
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

    int httpProtocol::decode(const char *buf, int len, int &uniq_no)
    {
        ostringstream oss;
        try
        {
            TC_HttpResponse http_rsp;
            if (!http_rsp.decode(buf, len))
            {
                return BM_PACKET_DECODE;
            }

            if (http_rsp.getStatus() == 0)
            {
                uniq_no = -1;
                return 0;
            }

            uniq_no = 1;
            return http_rsp.getStatus() == 200 ? 0 : http_rsp.getStatus();
        }
        catch (exception &e)
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
        char *pos_body = const_cast<char *>(strstr(buf, "\r\n\r\n"));
        if (pos_body == NULL)
        {
            return 0;
        }

        // 解析头部
        pos_body += 4;
        size_t headlen = (size_t)(pos_body - buf);
        if (headlen > len)
        {
            return BM_PACKET_ERROR;
        }

        TC_HttpResponse http_rsp;
        http_rsp.parseResponseHeaderString(buf, pos_body);
        if (http_rsp.getStatus() == 204 || http_rsp.getStatus() == 301 || http_rsp.getStatus() == 302)
        {
            // 直接返回
            return headlen;
        }

        size_t contentlen = 0;
        if (!http_rsp.getHeader("Content-Length").empty())
        {
            contentlen = http_rsp.getContentLength();
        }

        while (http_rsp.getHeader("Transfer-Encoding") == "chunked")
        {
            char *pos_chunk = strstr(pos_body, "\r\n");
            if (pos_chunk == NULL)
            {
                return 0;
            }

            //查找当前chunk的大小
            size_t bodylen = (int)(pos_chunk - buf + 2);
            string chunk_size = string(pos_body, (size_t)(pos_chunk - pos_body));
            int chunklen = strtol(chunk_size.c_str(), NULL, 16);
            if (chunklen <= 0)
            {
                return bodylen <= len ? bodylen : BM_PACKET_ERROR;
            }

            if (bodylen > len || (bodylen + chunklen + 2) > len)
            {
                return BM_PACKET_ERROR;
            }
            pos_body = pos_chunk + 2;
        }
        return headlen + contentlen;
    }
}; // namespace bm
