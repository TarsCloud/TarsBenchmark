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


const {
    adminRegPrx,
} = require('./rpc');
const {
    adminRegStruct
} = require('./rpc/struct');

const logger = require('../logger');

const AdminService = {};

AdminService.pingNode = async (nodeName) => {

    let ret = await adminRegPrx.pingNode(nodeName);

    if (ret.__return) {
        return ret.result;
    } else {
        logger.error(`ping node: ${nodeName} error`);
        console.log(`ping node: ${nodeName} error`);
        return false;

    }
};

AdminService.doCommand = async (targets, command) => {
    let rets = [];
    for (var i = 0, len = targets.length; i < len; i++) {
        let target = targets[i];
        let ret = {};
        try {
            ret = await adminRegPrx.notifyServer(target.application, target.serverName, target.nodeName, command);
        } catch (e) {
            ret = {
                __return: -1,
                result: e
            }
        }

        rets.push({
            application: target.application,
            server_name: target.serverName,
            node_name: target.nodeName,
            ret_code: ret.__return,
            err_msg: ret.result
        });
    }
    return rets;
};

AdminService.registerPlugin = async (name, name_en, obj, type, path) => {

    let conf = new adminRegStruct["PluginConf"];

    conf.name = name;
    conf.name_en = name_en;
    conf.obj = obj;
    conf.type = type;
    conf.path = path;

    let ret = await adminRegPrx.registerPlugin(conf);

    if (ret.__return == 0) {
        return 0;
    } else {
        logger.error(`registerPlugin: ${conf} error`);
        console.log(`registerPlugin: ${conf} error`);
        return -1;
    }
};

AdminService.hasDevAuth = async (application, server_name, uid) => {

    let ret = await adminRegPrx.hasDevAuth(application, server_name, uid);

    if (ret.__return == 0) {
        return ret.has;
    } else {
        logger.error(`hasDevAuth error`);
        console.log(`hasDevAuth error`);
        return false;
    }
};

AdminService.hasOpeAuth = async (application, server_name, uid) => {

    let ret = await adminRegPrx.hasOpeAuth(application, server_name, uid);

    if (ret.__return == 0) {
        return ret.has;
    } else {
        logger.error(`hasDevAuth error`);
        console.log(`hasDevAuth error`);
        return false;
    }
};

AdminService.hasAdminAuth = async (uid) => {

    let ret = await adminRegPrx.hasAdminAuth(uid);

    if (ret.__return == 0) {
        return ret.has;
    } else {
        logger.error(`hasAdminAuth error`);
        console.log(`hasAdminAuth error`);
        return false;
    }
};

AdminService.checkTicket = async (ticket) => {

    let ret = await adminRegPrx.checkTicket(ticket);

    if (ret.__return == 0) {
        return ret.uid;
    } else {
        logger.error(`checkTicket error`);
        return '';
    }
};


module.exports = AdminService;