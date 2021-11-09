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

#include "util/tc_file.h"
#include "util/tc_common.h"
#include "util/tc_hash_fun.h"
#include "json_protocol.h"

namespace bm
{
#define WRITENUM(st, vt, f, v)                             \
    if (f.type.find(st) == 0)                              \
    {                                                      \
        string s;                                          \
        vt val = TC_Common::strto<vt>(f.defval);           \
        try                                                \
        {                                                  \
            JsonInput::readJson(s, v);                     \
            val = TC_Common::strto<vt>(genRandomValue(s)); \
        }                                                  \
        catch (TC_Exception & e)                           \
        {                                                  \
            JsonInput::readJson(val, v, false);            \
        }                                                  \
        os.write(val, f.tag);                              \
        return 0;                                          \
    }

#define READINT(st, vt, vt2, f, fv)           \
    if (f.type.find(st) == 0)                 \
    {                                         \
        if (f.usigned)                        \
        {                                     \
            vt2 tmp = 0;                      \
            is.read(tmp, f.tag, false);       \
            return new JsonValueNum((int64_t)tmp, fv); \
        }                                     \
        else                                  \
        {                                     \
            vt tmp = 0;                       \
            is.read(tmp, f.tag, false);       \
            return new JsonValueNum((int64_t)tmp, fv); \
        }                                     \
    }

#define READFLOAT(st, vt, vt2, f, fv)           \
if (f.type.find(st) == 0)                 \
{                                         \
if (f.usigned)                        \
{                                     \
vt2 tmp = 0;                      \
is.read(tmp, f.tag, false);       \
return new JsonValueNum((double)tmp, fv); \
}                                     \
else                                  \
{                                     \
vt tmp = 0;                       \
is.read(tmp, f.tag, false);       \
return new JsonValueNum((double)tmp, fv); \
}                                     \
}

    IMPLEMENT_DYNCREATE(jsonProtocol, jsonProtocol)

    int jsonProtocol::initialize(int argc, char **argv)
    {
        // 支持命令
        licote_option_add("-S", NULL, "tars servant");
        licote_option_add("-M", NULL, "tars single interface name");
        licote_option_add("-C", "o", "tars single interface case");
        licote_option_init(argc, argv);

        _function = LICODE_GETSTR("-M", "");
        _servant = LICODE_GETSTR("-S", "");
        _timeout = LICODE_GETINT("-t", 3000);
        try
        {
            string file_pre = LICODE_GETSTR("-C", _function);
            if (!TC_File::isFileExist(file_pre + ".desc"))
            {
                licote_option_help("interface description file not exist\n");
            }

            if (!TC_File::isFileExist(file_pre + ".case"))
            {
                licote_option_help("interface json case file not exist\n");
            }

            parseCase(TC_File::load2str(file_pre + ".desc"), TC_File::load2str(file_pre + ".case"));
        }
        catch (exception &e)
        {
            string s = string("case datatype not match:") + e.what() + "\n";
            licote_option_help(s.c_str());
        }
        return BM_SUCC;
    }

    int jsonProtocol::initialize(const vector<string> &params)
    {
        if (params.size() != 5 && params.size() != 6)
        {
            return BM_INIT_PARAM;
        }

        _servant = TC_Common::trim(params[0]);
        _function = TC_Common::trim(params[1]);
        _timeout = TC_Common::strto<int>(params[2]);
        try
        {
            _os.reset();
            _random_flag = false;
            parseCase(params[3], params[4]);
            if (params.size() > 5)
            {
                parseField(JsonValueObjPtr::dynamicCast(TC_Json::getValue(params[5])), _resp_field);
            }
        }
        catch (exception &e)
        {
            return BM_PACKET_ERROR;
        }
        return BM_SUCC;
    }

    int jsonProtocol::parseCase(const string &in_param, const string &in_value)
    {
        parseField(JsonValueObjPtr::dynamicCast(TC_Json::getValue(in_param)), _para_field);
        _para_value = JsonValueObjPtr::dynamicCast(TC_Json::getValue(in_value));
        for (auto &field : _para_field)
        {
            encode(_os, field, _para_value->value[field.name]);
        }
        return 0;
    }

    void jsonProtocol::parseField(JsonValueObjPtr ptr, vector<JsonField> &field, int elem_num)
    {
        if (elem_num == -1 && ptr.get() == NULL)
        {
            return;
        }

        field.clear();
        for (auto &op : ptr->value)
        {
            vector<string> attr = TC_Common::sepstr<string>(op.first, "_");
            if (attr.size() >= 3)
            {
                JsonField f;
                size_t idx = attr.size() - (attr.back() == "u" ? 2 : 1);
                f.tag = TC_Common::strto<int>(attr[0]);
                f.type = TC_Common::trim(attr[idx]);
                f.name = TC_Common::trim(attr[1]);
                f.usigned = attr.back() == "u";
                for (size_t i = 2; i < idx; i++)
                {
                    f.name.append("_").append(attr[i]);
                }
                if (f.type == PT_STRUCT)
                {
                    parseField(JsonValueObjPtr::dynamicCast(op.second), f.child);
                }
                else if (f.type == PT_VECTOR)
                {
                    parseField(JsonValueObjPtr::dynamicCast(op.second), f.child, 1);
                }
                else if (f.type == PT_MAP)
                {
                    parseField(JsonValueObjPtr::dynamicCast(op.second), f.child, 2);
                }
                else
                {
                    JsonInput::readJson(f.defval, op.second);
                }
                field.push_back(f);
            }
        }

        if (elem_num != -1 && (int)field.size() != elem_num)
        {
            throw runtime_error("invalid element");
        }

        sort(field.begin(), field.end(), compField);
    }

    int jsonProtocol::encode(TarsOutputStream<BufferWriter> &os, JsonField &f, JsonValuePtr v)
    {
        WRITENUM(PT_BYTE, Short, f, v);
        WRITENUM(PT_SHORT, Int32, f, v);
        WRITENUM(PT_INT, Int64, f, v);
        WRITENUM(PT_LONG, Int64, f, v);
        WRITENUM(PT_FLOAT, Float, f, v);
        WRITENUM(PT_DOUBLE, Double, f, v);
        WRITENUM(PT_BOOLEAN, Bool, f, v);

        if (f.type.find(PT_STRING) == 0)
        {
            string s(f.defval);
            JsonInput::readJson(s, v, false);
            os.write(genRandomValue(s), f.tag);
        }
        else if (f.type.find(PT_VECTOR) == 0)
        {
            JsonValueArrayPtr av = JsonValueArrayPtr::dynamicCast(v);
            if (PT_BYTE.compare(f.child[0].type) == 0)
            {
                string sv;
                TarsWriteToHead(os, TarsHeadeSimpleList, f.tag);
                TarsWriteToHead(os, TarsHeadeChar, f.tag);
                if (NULL != v.get() && v->getType() == eJsonTypeString)
                {
                    sv = TC_Common::str2bin(JsonValueStringPtr::dynamicCast(v)->value);
                }
                else if (av.get() != NULL)
                {
                    for (size_t i = 0; i < av->value.size(); i++)
                    {
                        Char ch = 0x20;
                        JsonInput::readJson(ch, av->value[i], false);
                        sv.append(1, ch);
                    }
                }

                os.write(sv.length(), 0);
                TarsWriteTypeBuf(os, sv.c_str(), sv.length());
            }
            else
            {
                TarsWriteToHead(os, TarsHeadeList, f.tag);
                size_t arr_size = NULL == av.get() ? 0 : av->value.size();
                os.write(arr_size, 0);
                for (size_t i = 0; i < arr_size; ++i)
                {
                    encode(os, f.child[0], av->value[i]);
                }
            }
        }
        else if (f.type.find(PT_STRUCT) == 0)
        {
            JsonValueObjPtr ov = JsonValueObjPtr::dynamicCast(v);
            TarsWriteToHead(os, TarsHeadeStructBegin, f.tag);
            for (size_t i = 0; i < f.child.size(); ++i)
            {
                encode(os, f.child[i], ov.get() == NULL ? v : ov->value[f.child[i].name]);
            }
            TarsWriteToHead(os, TarsHeadeStructEnd, 0);
        }
        else if (f.type.find(PT_MAP) == 0)
        {
            JsonValueObjPtr ov = JsonValueObjPtr::dynamicCast(v);
            TarsWriteToHead(os, TarsHeadeMap, f.tag);
            if (ov.get() != NULL)
            {
                os.write(ov->value.size(), 0);
                for (auto &elem : ov->value)
                {
                    encode(os, f.child[0], new JsonValueString(elem.first));
                    encode(os, f.child[1], elem.second);
                }
            }
            else
            {
                os.write(0, 0);
            }
        }
        return 0;
    }

    JsonValuePtr jsonProtocol::decode(TarsInputStream<BufferReader> &is, JsonField &f)
    {
        READINT(PT_BYTE, Char, Short, f, true);
        READINT(PT_SHORT, Short, Int32, f, true);
        READINT(PT_INT, Int32, Int64, f, true);
        READINT(PT_LONG, Int64, Int64, f, true);
        READFLOAT(PT_FLOAT, Float, Float, f, false);
        READFLOAT(PT_DOUBLE, Double, Double, f, false);

        if (f.type.find(PT_STRING) == 0)
        {
            string tmp;
            is.read(tmp, f.tag, false);
            return new JsonValueString(tmp);
        }
        else if (f.type.find(PT_BOOLEAN) == 0)
        {
            JsonValueBooleanPtr bptr = new JsonValueBoolean();
            is.read(bptr->value, f.tag, false);
            return bptr;
        }
        else if (f.type.find(PT_VECTOR) == 0)
        {
            JsonValueArrayPtr vp = new JsonValueArray();
            if (is.skipToTag(f.tag))
            {
                DataHead h;
                h.readFrom(is);
                if (h.getType() == DataHead::eList)
                {
                    Int32 size;
                    is.read(size, 0);
                    for (int i = 0; i < size; ++i)
                    {
                        vp->value.push_back(decode(is, f.child[0]));
                    }
                }
                else if (h.getType() == DataHead::eSimpleList)
                {
                    h.readFrom(is);
                    if (h.getType() == DataHead::eChar)
                    {
                        Int32 size;
                        vector<Char> vb;
                        is.read(size, 0);
                        is.readBuf(vb, size);
                        string sv = TC_Common::bin2str(&vb[0], vb.size());
                        return new JsonValueString(sv);
                    }
                    else
                    {
                        throw runtime_error("parse vector fail");
                    }
                }
            }
            return vp;
        }
        else if (f.type.find(PT_MAP) == 0)
        {
            JsonValueObjPtr mp = new JsonValueObj();
            if (is.skipToTag(f.tag))
            {
                DataHead h;
                h.readFrom(is);
                if (h.getType() == DataHead::eMap)
                {
                    Int32 size;
                    is.read(size, 0);
                    for (int i = 0; i < size; ++i)
                    {
                        JsonValuePtr p = decode(is, f.child[0]);
                        if (p.get() != NULL && p->getType() == eJsonTypeNum)
                        {
                            mp->value[TC_Common::tostr(JsonValueNumPtr::dynamicCast(p)->value)] = decode(is, f.child[1]);
                        }
                        else if (p.get() != NULL && p->getType() == eJsonTypeString)
                        {
                            mp->value[JsonValueStringPtr::dynamicCast(p)->value] = decode(is, f.child[1]);
                        }
                        else
                        {
                            throw runtime_error("decode map fail");
                        }
                    }
                }
            }
            return mp;
        }
        else if (f.type.find(PT_STRUCT) == 0)
        {
            JsonValueObjPtr sp = new JsonValueObj();
            if (is.skipToTag(f.tag))
            {
                DataHead h;
                h.readFrom(is);
                if (h.getType() == DataHead::eStructBegin)
                {
                    for (size_t i = 0; i < f.child.size(); ++i)
                    {
                        sp->value[f.child[i].name] = decode(is, f.child[i]);
                    }
                    is.skipToStructEnd();
                }
            }
            return sp;
        }

        throw runtime_error("unknow type");
        return NULL;
    }

    int jsonProtocol::encode(char *buf, int &len, int &uniq_no)
    {
        ostringstream oss;
        try
        {
            TarsOutputStream<BufferWriter> os;
            if (_random_flag == true)
            {
                _os.reset();
                for (auto &field : _para_field)
                {
                    encode(_os, field, _para_value->value[field.name]);
                }
            }

            RequestPacket req;
            req.iRequestId = uniq_no;
            req.iVersion = 1;
            req.cPacketType = 0;
            req.iTimeout = _timeout;
            req.sServantName = _servant;
            req.sFuncName = _function;
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

    int jsonProtocol::decode(const char *buf, int len, int &uniq_no, string* out)
    {
        ostringstream oss;
        try
        {
            TarsInputStream<BufferReader> is, isf;
            is.setBuffer(buf + 4, len - 4);

            ResponsePacket rsp;
            rsp.readFrom(is);

            if (out != NULL)
            {
                isf.setBuffer(rsp.sBuffer);
                JsonValueObjPtr jout = new JsonValueObj;
                jout->value["tarsret"] = new JsonValueNum(decodeReturn(isf), true);
                for (auto &field : _resp_field)
                {
                    jout->value[field.name] = decode(isf, field);
                }

                *out = TC_Json::writeValue(jout);
            }

            uniq_no = rsp.iRequestId;
            return rsp.iRet;
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

    int jsonProtocol::input(const char *buf, size_t len)
    {
        size_t head_len = ntohl(*(uint32_t *)(buf));
        if ((size_t)len < sizeof(uint32_t) || head_len < sizeof(int) || len < head_len)
        {
            return 0;
        }
        return (int)head_len;
    }
}; // namespace bm
