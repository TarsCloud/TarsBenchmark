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

const client = require("@tars/rpc/protal.js").client;

if (!process.env.TARS_CONFIG) {
    client.setProperty("locator", "tars.tarsregistry.QueryObj@tcp -h 127.0.0.1 -p 17890");
}

const AdminRegProxy = require("./proxy/AdminRegProxy");
const BenchmarkAdminProxy = require("./proxy/BenchmarkAdminProxy");
const BenchmarkNode = require("./proxy/BenchmarkNodeTars");
const webConf = require("../../config/webConf");

const {
    RPCClientPrx,
    RPCStruct
} = require('./service');

module.exports = {

    adminRegPrx: RPCClientPrx(client, AdminRegProxy, 'tars', 'AdminReg', 'tars.tarsAdminRegistry.AdminRegObj'),
    adminRegStruct: RPCStruct(AdminRegProxy, 'tars'),

    benchmarkPrx: RPCClientPrx(client, BenchmarkAdminProxy, 'bm', 'Admin', webConf.benchmarkObj),
    benchmarkStruct: RPCStruct(BenchmarkAdminProxy, 'bm'),
    benchmarkNodeStruct: RPCStruct(BenchmarkNode, 'bm'),

    client: client,
};