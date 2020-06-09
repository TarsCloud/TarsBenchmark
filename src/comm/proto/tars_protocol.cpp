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
#include "tars_protocol.h"
#include "util/tc_common.h"

namespace bm
{
    IMPLEMENT_DYNCREATE(tarsProtocol, tarsProtocol)

    int tarsProtocol::initialize(int argc, char** argv)
    {
        // 支持命令
        licote_option_add("-S", NULL,  "tars servant");
        licote_option_add("-M", NULL,  "tars single interface name");
        licote_option_add("-C", NULL,  "tars single interface case");
        licote_option_init(argc, argv);

        _servant  = LICODE_GETSTR("-S", "");
        _function = LICODE_GETSTR("-M", "");
        _timeout  = LICODE_GETINT("-t", 3000);

        ifstream ifs(licote_option_get("-C"));
        if (!ifs.is_open())
        {
            licote_option_help("case file open failed\n");
        }

        int parse_state = 0;
        string s_line, s_param, s_value;
        while (getline(ifs, s_line))
        {
            string ss = TC_Common::trim(s_line);
            if (ss.empty())
            {
                continue;
            }

            // 以#为开始的注释
            if (parse_state != 2 && ss.find_first_of("#") == 0)
            {
                continue;
            }

            if (parse_state == 0)
            {
                s_param = ss;
                parse_state = 1;
                continue;
            }

            parse_state = 2;
            s_value.append(ss + "\n");
        }

        try
        {
            parseCase(s_param, s_value);
        }
        catch (exception& e)
        {
            string s = string(e.what()) + "\n";
            licote_option_help(s.c_str());
        }

        return BM_SUCC;
    }

    int tarsProtocol::initialize(const vector<string>& params)
    {
        if (params.size() != 5)
        {
            return BM_INIT_PARAM;
        }

        _servant  = TC_Common::trim(params[0]);
        _function = TC_Common::trim(params[1]);
        _timeout  = TC_Common::strto<int>(params[2]);
        try
        {
            _os.reset();
            _random_flag = false;
            parseCase(params[3], params[4]);
        }
        catch (exception& e)
        {
            return BM_PACKET_ERROR;
        }
        return BM_SUCC;
    }

    int tarsProtocol::parseCase(const string& in_param, const string& in_value)
    {
        _para_list = TC_Common::sepstr<string>(in_param, "|");
        _para_vals = TC_Common::sepstr<string>(escapeStr(in_value), "\n");
        if (_para_vals.size() !=  _para_list.size())
        {
            throw runtime_error("case parameter not match");
        }

        for (size_t ii = 0; ii < _para_list.size(); ii++)
        {
            encode(_os, _para_list[ii], _para_vals[ii], ii + 1);
        }
        return 0;
    }

    string tarsProtocol::getType(const string& type)
    {
        string::size_type l = type.find_first_of('<');
        string::size_type r = type.find_last_of('>');
        if (l == string::npos || r == string::npos)
        {
            throw runtime_error("invalid type" + type);
        }
        return TC_Common::trim(type.substr(l + 1, r - l - 1));
    }

    string tarsProtocol::getMapKey(const string& type)
    {
        int j = 0, p = 0;
        string s = getType(type);
        for (int i = 0; i < (int)s.size(); ++i)
        {
            if(s.at(i) == '<')
            {
                ++j;
            }
            else if (s.at(i) == '>')
            {
                --j;
            }
            else if (s.at(i) == ',' && 0 == j)
            {
                p = i;
            }
        }

        if (0 == p || 0 != j)
        {
            throw runtime_error("invalid map key:" + type);
        }

        return TC_Common::trim(s.substr(0, p));
    }

    Field tarsProtocol::getField(const string& type, int tag, bool require)
    {
        int p = 0;
        Field field;
        string short_type("");
        for (int i = 0; i < (int)type.size(); ++i)
        {
            if (type.at(i) == '<')
            {
                if (0 == p)
                {
                    p = i;
                }
            }
        }

        if (0 == p)
        {
            p = type.size();
        }

        short_type = TC_Common::trim(type.substr(0, p));
        vector<string> attr =TC_Common::sepstr<string>(short_type, " ");
        if (attr.size() == 3)
        {
            int attr_tag = TC_Common::strto<int>(attr[0]);
            field.tag = (attr_tag == 0 ? tag : attr_tag);
            field.type = (attr[2] + short_type);
            field.require = (string("require").compare(attr[1]) == 0 ? true : false);
        }
        else
        {
            field.tag = tag;
            field.type = type;
            field.require = require;
        }
        return field;
    }

    string tarsProtocol::getMapValue(const string& type)
    {
        int j = 0;
        int p = 0;

        string s = getType(type);
        for (int i = 0; i < (int)s.size(); ++i)
        {
            if (s.at(i) == '<')
            {
                ++j;
            }
            else if (s.at(i) == '>')
            {
                --j;
            }
            else if (s.at(i) == ',' && 0 == j)
            {
                p = i;
            }
        }

        if (p == 0 || 0 != j)
        {
            throw runtime_error("invalid map val" + type);
        }

        return TC_Common::trim(s.substr(p + 1, s.size() - p - 1));
    };

    vector<string> tarsProtocol::getArray(const string& v)
    {
        int pos = 0;
        int j = 0;
        int k = 0;

        vector<string> vs;
        for (int i = 0; i < (int)v.size(); ++i)
        {
            if (v.at(i) == '<')
            {
                if (0 == j)
                {
                    pos = i;
                }
                ++j;
            }
            else if (v.at(i) == '>')
            {
                if (j < 1)
                {
                    throw runtime_error("array need <");
                }
                --j;
                if (0 == j)
                {
                    vs.push_back(TC_Common::trim(v.substr(pos + 1, i - pos - 1)));
                }
            }
            else if (v.at(i) == ',' && k == 0)
            {
                if (1 == j)
                {
                    vs.push_back(TC_Common::trim(v.substr(pos + 1, i - pos - 1)));
                    pos = i;
                }
            }
            else if (v.at(i) == '[')
            {
                if (j < 1)
                {
                    throw runtime_error("array(map) need <");
                }
                ++k;
            }
            else if (v.at(i) == ']')
            {
                if (k < 1 || j < 1)
                {
                    throw runtime_error("array(map) need <>");
                }
                --k;
            }
        }

        if (0 != j || 0 != k)
        {
            throw runtime_error("invalid array");
        }
        return vs;
    }

    int tarsProtocol::encode(TarsOutputStream<BufferWriter> &os, const string& stype, const string& sval, int tag, bool usigned)
    {
        string type = TC_Common::trim(stype);
        string val  = TC_Common::trim(sval);
        if (type.find(PT_VOID) == 0)
        {
            return 0;
        }
        else if (type.find(PT_BOOLEAN) == 0)
        {
            os.write(TC_Common::strto<Bool>(genRandomValue(val, false), "false"), tag);
        }
        else if (type.find(PT_BYTE) == 0)
        {
            if (usigned)
            {
                os.write(TC_Common::strto<Short>(genRandomValue(val), "0"), tag);
            }
            else
            {
                os.write(TC_Common::strto<Char>(genRandomValue(val), "0"), tag);
            }
        }
        else if (type.find(PT_SHORT) == 0)
        {
            if (usigned)
            {
                os.write(TC_Common::strto<Int32>(genRandomValue(val), "0"), tag);
            }
            else
            {
                os.write(TC_Common::strto<Short>(genRandomValue(val), "0"), tag);
            }
        }
        else if (type.find(PT_INT) == 0)
        {
            if (usigned)
            {
                os.write(TC_Common::strto<Int64>(genRandomValue(val), "0"), tag);
            }
            else
            {
                os.write(TC_Common::strto<Int32>(genRandomValue(val), "0"), tag);
            }
        }
        else if (type.find(PT_UNSIGNED) == 0)
        {
            encode(os, type.substr(PT_UNSIGNED.size(), type.size() - PT_UNSIGNED.size()), genRandomValue(val), tag, true);
        }
        else if (type.find(PT_FLOAT) == 0)
        {
            os.write(TC_Common::strto<Float>(genRandomValue(val, false), "0"), tag);
        }
        else if (type.find(PT_LONG) == 0)
        {
            os.write(TC_Common::strto<Int64>(genRandomValue(val), "0"), tag);
        }
        else if (type.find(PT_DOUBLE) == 0)
        {
            os.write(TC_Common::strto<Double>(genRandomValue(val, false), "0"), tag);
        }
        else if (type.find(PT_STRING) == 0)
        {
            os.write(genRandomValue(unescapeStr(val), false), tag);
        }
        else if (type.find(PT_VECTOR) == 0)
        {
            vector<string> vs = getArray(val);
            string sub_type = getType(type);

            if (PT_BYTE.compare(sub_type) == 0)
            {
                TarsWriteToHead(os, TarsHeadeSimpleList, tag);
                TarsWriteToHead(os, TarsHeadeChar, tag);
                os.write(vs.size(),0);
                for (int i = 0; i < (int)vs.size(); ++i)
                {
                    encode(os, sub_type, vs.at(i), 0, false);
                }
            }
            else
            {
                TarsWriteToHead(os, TarsHeadeList, tag);
                os.write(vs.size(), 0);
                for (int i = 0; i < (int)vs.size(); ++i)
                {
                    encode(os, sub_type, vs.at(i), 0, false);
                }
            }
        }
        else if (type.find(PT_STRUCT) == 0)
        {
            vector<string> vt = getArray(type);
            vector<string> vv = getArray(val);
            if (vt.size() != vv.size())
            {
                throw runtime_error("invalid struct");
            }

            TarsWriteToHead(os, TarsHeadeStructBegin, tag);
            for (int i = 0, ttag = 0; i < (int)vt.size(); ++i)
            {
                Field field = getField(vt.at(i), ttag, true);
                encode(os, field.type, vv.at(i), field.tag, false);
                ttag = field.tag + 1;
            }
            TarsWriteToHead(os, TarsHeadeStructEnd, 0);
        }
        else if (type.find(PT_MAP) == 0)
        {
            map<string, string> mp;
            int pos = 0;
            int j = 0;
            int k = 0;
            string s1("");
            string s2("");

            for (int i = 0; i < (int)val.size(); ++i)
            {
                if (val.at(i) == '[')
                {
                    if (0 == j)
                    {
                        pos = i;
                    }
                    ++j;
                }
                else if (val.at(i) == ']' && k == 0)
                {
                    if (j < 1)
                    {
                        throw runtime_error("invalid map");
                    }
                    --j;
                    if (0 == j)
                    {
                        s2 = TC_Common::trim(val.substr(pos + 1, i - pos - 1));
                        mp[s1] = s2;
                        break;
                    }
                }
                else if (val.at(i) == '=' && k == 0)
                {
                    if (j == 1)
                    {
                        s1 =  TC_Common::trim(val.substr(pos + 1, i - pos - 1));
                        pos = i;
                    }
                }
                else if (val.at(i) == ',' && k == 0)
                {
                    if (j == 1)
                    {
                        s2 = TC_Common::trim(val.substr(pos + 1, i - pos - 1));
                        mp[s1] = s2;
                        pos = i;
                    }
                }
                else if (val.at(i) == '<')
                {
                    if (j < 1)
                    {
                        throw runtime_error("invalid map(unknown <)");
                    }
                    ++k;
                }
                else if (val.at(i) == '>')
                {
                    if (k < 1 || j < 1)
                    {
                        throw runtime_error("invalid map(unknown >)");
                    }
                    --k;
                }
            }

            TarsWriteToHead(os, TarsHeadeMap, tag);
            os.write(mp.size(), 0);

            map<string, string>::const_iterator it = mp.begin();
            string left_type  = getMapKey(type);
            string right_type = getMapValue(type);
            while (it != mp.end())
            {
                encode(os, left_type, TC_Common::trim(it->first), 0, false);
                encode(os, right_type, TC_Common::trim(it->second), 1, false);
                ++it;
            }
        }
        else
        {
            throw runtime_error(string("unknown type: ") + type);
        }
        return 0;
    }

    string tarsProtocol::decode(TarsInputStream<BufferReader> &is, const string& sType, int tag, bool require, bool usigned)
    {
        string type = TC_Common::trim(sType);
        string s("");
        if (type.find(PT_VOID) == 0)
        {
            return s;
        }
        else if (type.find(PT_SHORT) == 0)
        {
            if (usigned)
            {
                int tmp = 0;
                is.read(tmp, tag, require);
                s = TC_Common::tostr(tmp);
            }
            else
            {
                short tmp = 0;
                is.read(tmp, tag, require);
                s = TC_Common::tostr(tmp);
            }
        }
        else if (type.find(PT_BYTE) == 0)
        {
            if (usigned)
            {
                short tmp = 0;
                is.read(tmp, tag, require);
                s = TC_Common::tostr(tmp);
            }
            else
            {
                char tmp;
                is.read(tmp, tag, require);
                s = TC_Common::tostr(tmp);
            }
        }
        else if (type.find(PT_INT) == 0)
        {
            if (usigned)
            {
                long tmp = 0L;
                is.read(tmp, tag, require);
                s = TC_Common::tostr(tmp);
            }
            else
            {
                int i = 0;
                is.read(i, tag, require);
                s = TC_Common::tostr(i);
            }
        }
        else if (type.find(PT_FLOAT) == 0)
        {
            float tmp = 0.0f;
            is.read(tmp, tag, require);
            s = TC_Common::tostr(tmp);
        }
        else if (type.find(PT_LONG) == 0)
        {
            long tmp = 0L;
            is.read(tmp, tag, require);
            s = TC_Common::tostr(tmp);
        }
        else if (type.find(PT_DOUBLE) == 0)
        {
            double tmp = 0.0;
            is.read(tmp, tag, require);
            s = TC_Common::tostr(tmp);
        }
        else if (type.find(PT_STRING) == 0)
        {
            is.read(s, tag, require);
        }
        else if (type.find(PT_UNSIGNED) == 0)
        {
            s = decode(is, type.substr(PT_UNSIGNED.size(), type.size() - PT_UNSIGNED.size()), tag, require, true);
        }
        else if (type.find(PT_VECTOR) == 0)
        {
            if (is.skipToTag(tag))
            {
                DataHead h;
                h.readFrom(is);
                if (h.getType() == DataHead::eList)
                {
                    Int32 size;
                    is.read(size, 0);
                    assert(size >= 0);
                    string sub_type = getType(type);
                    s += "<";
                    for (int i = 0; i < size; ++i)
                    {
                        s += decode(is, sub_type, 0, require, false);
                        if (i != size - 1)
                        {
                            s += ",";
                        }
                        else
                        {
                            s += ">";
                        }
                    }
                }
                else if (h.getType() == DataHead::eSimpleList)
                {
                    h.readFrom(is);
                    if (h.getType() == DataHead::eChar)
                    {
                        Int32 size;
                        is.read(size, 0);
                        assert(size >= 0);
                        string sub_type = "char";
                        s += "<";
                        for (int i = 0; i < size; ++i)
                        {
                            s += decode(is, sub_type, 0, require, false);
                            if (i != size - 1)
                            {
                                s += ",";
                            }
                            else
                            {
                                s += ">";
                            }
                        }
                    }
                    else
                    {
                        throw runtime_error("parse vector fail");
                    }
                }
            }
        }
        else if (type.find(PT_MAP) == 0)
        {
            if (is.skipToTag(tag))
            {
                DataHead h;
                h.readFrom(is);
                if (h.getType() == DataHead::eMap)
                {
                    Int32 size;
                    is.read(size, 0);
                    assert(size >= 0);
                    s += "[";
                    for (int i = 0; i < size; ++i)
                    {
                        s += decode(is, getMapKey(type), 0, require, false);
                        s += "=";
                        s += decode(is, getMapValue(type), 1, require, false);
                        if (i != size - 1)
                        {
                            s += ",";
                        }
                        else
                        {
                            s += "]";
                        }
                    }
                }
            }
        }
        else if (type.find(PT_STRUCT) == 0)
        {
            s += "<";
            vector<string> vt = getArray(type);

            if (is.skipToTag(tag))
            {
                DataHead h;
                h.readFrom(is);
                if (h.getType() == DataHead::eStructBegin)
                {
                    int struct_tag = 0;
                    for (int i = 0; i < (int)vt.size(); ++i)
                    {
                        string type_desc = vt.at(i);
                        Field field = getField(type_desc, struct_tag, require);
                        string tmp = decode(is, field.type, field.tag, field.require, false);
                        s += tmp + ((i != int(vt.size() - 1)) ? "," : ">");
                        struct_tag = field.tag + 1;
                    }
                    is.skipToStructEnd();
                }
            }
        }
        return s;
    }

    int tarsProtocol::encode(char *buf, int& len, int& uniqId)
    {
        ostringstream oss;
        try
        {
            TarsOutputStream<BufferWriter> os;
            if (_para_list.size() != _para_vals.size())
            {
                return BM_PACKET_PARAM;
            }

            if (_random_flag)
            {
                _os.reset();
                for (size_t ii = 0; ii < _para_list.size(); ii++)
                {
                    encode(_os, _para_list[ii], _para_vals[ii], ii + 1);
                }
            }

            RequestPacket req;
            req.iRequestId   = uniqId;
            req.iVersion     = 1;
            req.cPacketType  = 0;
            req.iTimeout     = _timeout;
            req.sServantName = _servant;
            req.sFuncName    = _function;
            req.context["AppName"] = "bmClient";
            req.status["AppName"] = "bmClient";
            req.sBuffer = _os.getByteBuffer();
            req.writeTo(os);

            if ((size_t)len < (os.getLength() + 4))
            {
                return os.getLength() + 4;
            }

            len = sizeof(Int32) + os.getLength();
            Int32 iHeaderLen = htonl(sizeof(Int32) + os.getLength());
            memcpy(buf, &iHeaderLen, sizeof(Int32));
            memcpy(buf + sizeof(Int32), os.getBuffer(), os.getLength());
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

    int tarsProtocol::decode(const char *buf, int len, int& uniqId)
    {
        ostringstream oss;
        try
        {
            TarsInputStream<BufferReader> is;
            is.setBuffer(buf + 4, len - 4);

            ResponsePacket rsp;
            rsp.readFrom(is);

            uniqId = rsp.iRequestId;
            return rsp.iRet;
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

    string tarsProtocol::escapeStr(const string& src)
    {
        string dst = src;
        dst = TC_Common::replace(dst, "\\,", LABEL_ASCII_2C);
        dst = TC_Common::replace(dst, "\\<", LABEL_ASCII_3C);
        dst = TC_Common::replace(dst, "\\>", LABEL_ASCII_3E);
        return dst;
    }

    string tarsProtocol::unescapeStr(const string& src)
    {
        string dst = src;
        dst = TC_Common::replace(dst, LABEL_ASCII_2C, ",");
        dst = TC_Common::replace(dst, LABEL_ASCII_3C, "<");
        dst = TC_Common::replace(dst, LABEL_ASCII_3E, ">");
        return dst;
    }

    int tarsProtocol::input(const char *buf, size_t len)
    {
        size_t head_len = ntohl(*(uint32_t *)(buf));
        if ((size_t)len < sizeof(uint32_t) || head_len < sizeof(int) || len < head_len)
        {
            return 0;
        }
        return (int)head_len;
    }
};
