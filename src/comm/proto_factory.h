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
#ifndef _PROTO_FACTORY_H_
#define _PROTO_FACTORY_H_

#include <map>
#include <stdexcept>
#include "protocol.h"
using namespace std;

namespace bm
{
    class ProtoFactory
    {
    public:
        ProtoFactory() { _protos.clear(); }
        virtual ~ProtoFactory() { destroyObject(); }

        Protocol* get(const string& name, int argc, char** argv)
        {
            Protocol* cmd = NULL;
            if (_protos.find(name) == _protos.end())
            {
                TC_DYN_Object* obj = TC_DYN_CreateObject(name.c_str());
                if(!obj)
                {
                    throw runtime_error("protocol not existed:" + name);
                }

                if ((cmd = dynamic_cast<Protocol*>(obj)) == NULL)
                {
                    throw runtime_error("null protocol:" + name);
                }

                if (argc > 0 && argv != NULL)
                {
                    cmd->initialize(argc, argv);
                }

                _protos[name] = obj;
            }

            if ((cmd = dynamic_cast<Protocol*>(_protos[name])) == NULL)
            {
                throw runtime_error("null command:" + name);
            }

            return cmd;
        }

        void destroyObject(void)
        {
            for (map<string, TC_DYN_Object*>::iterator itm =  _protos.begin(); itm != _protos.end(); )
            {
                delete itm->second;
                _protos.erase(itm++);
            }
        }

    private:
        map<string, TC_DYN_Object*>    _protos;
    };
};
/////////////////////////////////////////////////////////////////
#endif
