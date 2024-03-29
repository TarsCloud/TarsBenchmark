// **********************************************************************
// Parsed By TarsParser(3.0.10), Generated By tools(20200627)
// TarsParser Maintained By <TARS> and tools Maintained By <superzheng>
// Generated from "AdminReg.tars" by Client Mode
// **********************************************************************

/* eslint-disable */

"use strict";

var assert    = require("assert");
var TarsStream = require("@tars/stream");
var TarsError  = require("@tars/rpc").error;

var _hasOwnProperty = Object.prototype.hasOwnProperty;
var _makeError = function (data, message, type) {
    var error = new Error(message || "");
    error.request = data.request;
    error.response = {
        "costtime" : data.request.costtime
    };
    if (type === TarsError.CLIENT.DECODE_ERROR) {
        error.name = "DECODE_ERROR";
        error.response.error = {
            "code" : type,
            "message" : message
        };
    } else {
        error.name = "RPC_ERROR";
        error.response.error = data.error;
    }
    return error;
};

var tars = tars || {};
module.exports.tars = tars;

tars.AdminRegProxy = function () {
    this._name    = undefined;
    this._worker  = undefined;
};

tars.AdminRegProxy.prototype.setTimeout = function (iTimeout) {
    this._worker.timeout = iTimeout;
};

tars.AdminRegProxy.prototype.getTimeout = function () {
    return this._worker.timeout;
};

tars.AdminRegProxy.prototype.setVersion = function (iVersion) {
    this._worker.version = iVersion;
};

tars.AdminRegProxy.prototype.getVersion = function () {
    return this._worker.version;
};

tars.tarsErrCode = {
    "EM_TARS_NODE_NOT_REGISTRY_ERR" : 1001,
    "EM_TARS_CALL_NODE_TIMEOUT_ERR" : 1002,
    "EM_TARS_LOAD_SERVICE_DESC_ERR" : 1003,
    "EM_TARS_SERVICE_STATE_ERR" : 1004,
    "EM_TARS_REQ_ALREADY_ERR" : 1005,
    "EM_TARS_INVALID_IP_ERR" : 1006,
    "EM_TARS_PARAMETER_ERR" : 1007,
    "EM_TARS_OTHER_ERR" : 1008,
    "EM_TARS_GET_PATCH_FILE_ERR" : 1009,
    "EM_TARS_PREPARE_ERR" : 1010,
    "EM_TARS_CAN_NOT_EXECUTE" : 1011,
    "EM_TARS_NODE_NO_CONNECTION" : 1012,
    "EM_TARS_UNKNOWN_ERR" : -1,
    "EM_TARS_SUCCESS" : 0
};
tars.tarsErrCode._classname = "tars.tarsErrCode";
tars.tarsErrCode._write = function(os, tag, val) { return os.writeInt32(tag, val); };
tars.tarsErrCode._read  = function(is, tag, def) { return is.readInt32(tag, true, def); };

tars.PluginConf = function() {
    this.name = "";
    this.name_en = "";
    this.obj = "";
    this.type = 0;
    this.path = "";
    this.k8s = true;
    this._classname = "tars.PluginConf";
};
tars.PluginConf._classname = "tars.PluginConf";
tars.PluginConf._write = function (os, tag, value) { os.writeStruct(tag, value); };
tars.PluginConf._read  = function (is, tag, def) { return is.readStruct(tag, true, def); };
tars.PluginConf._readFrom = function (is) {
    var tmp = new tars.PluginConf;
    tmp.name = is.readString(0, false, "");
    tmp.name_en = is.readString(1, false, "");
    tmp.obj = is.readString(2, false, "");
    tmp.type = is.readInt32(3, false, 0);
    tmp.path = is.readString(4, false, "");
    tmp.k8s = is.readBoolean(5, false, true);
    return tmp;
};
tars.PluginConf.prototype._writeTo = function (os) {
    os.writeString(0, this.name);
    os.writeString(1, this.name_en);
    os.writeString(2, this.obj);
    os.writeInt32(3, this.type);
    os.writeString(4, this.path);
    os.writeBoolean(5, this.k8s);
};
tars.PluginConf.prototype._equal = function () {
    assert.fail("this structure not define key operation");
};
tars.PluginConf.prototype._genKey = function () {
    if (!this._proto_struct_name_) {
        this._proto_struct_name_ = "STRUCT" + Math.random();
    }
    return this._proto_struct_name_;
};
tars.PluginConf.prototype.toObject = function() { 
    return {
        "name" : this.name,
        "name_en" : this.name_en,
        "obj" : this.obj,
        "type" : this.type,
        "path" : this.path,
        "k8s" : this.k8s
    };
};
tars.PluginConf.prototype.readFromObject = function(json) { 
    _hasOwnProperty.call(json, "name") && (this.name = json.name);
    _hasOwnProperty.call(json, "name_en") && (this.name_en = json.name_en);
    _hasOwnProperty.call(json, "obj") && (this.obj = json.obj);
    _hasOwnProperty.call(json, "type") && (this.type = json.type);
    _hasOwnProperty.call(json, "path") && (this.path = json.path);
    _hasOwnProperty.call(json, "k8s") && (this.k8s = json.k8s);
    return this;
};
tars.PluginConf.prototype.toBinBuffer = function () {
    var os = new TarsStream.TarsOutputStream();
    this._writeTo(os);
    return os.getBinBuffer();
};
tars.PluginConf.new = function () {
    return new tars.PluginConf();
};
tars.PluginConf.create = function (is) {
    return tars.PluginConf._readFrom(is);
};

tars.AuthConf = function() {
    this.flag = "";
    this.role = "";
    this.uid = "";
    this._classname = "tars.AuthConf";
};
tars.AuthConf._classname = "tars.AuthConf";
tars.AuthConf._write = function (os, tag, value) { os.writeStruct(tag, value); };
tars.AuthConf._read  = function (is, tag, def) { return is.readStruct(tag, true, def); };
tars.AuthConf._readFrom = function (is) {
    var tmp = new tars.AuthConf;
    tmp.flag = is.readString(0, false, "");
    tmp.role = is.readString(1, false, "");
    tmp.uid = is.readString(2, false, "");
    return tmp;
};
tars.AuthConf.prototype._writeTo = function (os) {
    os.writeString(0, this.flag);
    os.writeString(1, this.role);
    os.writeString(2, this.uid);
};
tars.AuthConf.prototype._equal = function () {
    assert.fail("this structure not define key operation");
};
tars.AuthConf.prototype._genKey = function () {
    if (!this._proto_struct_name_) {
        this._proto_struct_name_ = "STRUCT" + Math.random();
    }
    return this._proto_struct_name_;
};
tars.AuthConf.prototype.toObject = function() { 
    return {
        "flag" : this.flag,
        "role" : this.role,
        "uid" : this.uid
    };
};
tars.AuthConf.prototype.readFromObject = function(json) { 
    _hasOwnProperty.call(json, "flag") && (this.flag = json.flag);
    _hasOwnProperty.call(json, "role") && (this.role = json.role);
    _hasOwnProperty.call(json, "uid") && (this.uid = json.uid);
    return this;
};
tars.AuthConf.prototype.toBinBuffer = function () {
    var os = new TarsStream.TarsOutputStream();
    this._writeTo(os);
    return os.getBinBuffer();
};
tars.AuthConf.new = function () {
    return new tars.AuthConf();
};
tars.AuthConf.create = function (is) {
    return tars.AuthConf._readFrom(is);
};

tars.AuthConfList = function() {
    this.auths = new TarsStream.List(tars.AuthConf);
    this._classname = "tars.AuthConfList";
};
tars.AuthConfList._classname = "tars.AuthConfList";
tars.AuthConfList._write = function (os, tag, value) { os.writeStruct(tag, value); };
tars.AuthConfList._read  = function (is, tag, def) { return is.readStruct(tag, true, def); };
tars.AuthConfList._readFrom = function (is) {
    var tmp = new tars.AuthConfList;
    tmp.auths = is.readList(0, false, TarsStream.List(tars.AuthConf));
    return tmp;
};
tars.AuthConfList.prototype._writeTo = function (os) {
    os.writeList(0, this.auths);
};
tars.AuthConfList.prototype._equal = function () {
    assert.fail("this structure not define key operation");
};
tars.AuthConfList.prototype._genKey = function () {
    if (!this._proto_struct_name_) {
        this._proto_struct_name_ = "STRUCT" + Math.random();
    }
    return this._proto_struct_name_;
};
tars.AuthConfList.prototype.toObject = function() { 
    return {
        "auths" : this.auths.toObject()
    };
};
tars.AuthConfList.prototype.readFromObject = function(json) { 
    _hasOwnProperty.call(json, "auths") && (this.auths.readFromObject(json.auths));
    return this;
};
tars.AuthConfList.prototype.toBinBuffer = function () {
    var os = new TarsStream.TarsOutputStream();
    this._writeTo(os);
    return os.getBinBuffer();
};
tars.AuthConfList.new = function () {
    return new tars.AuthConfList();
};
tars.AuthConfList.create = function (is) {
    return tars.AuthConfList._readFrom(is);
};

var __tars_AdminReg$checkTicket$IF = {
    "name" : "checkTicket",
    "return" : "int32",
    "arguments" : [{
        "name" : "ticket",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "uid",
        "class" : "string",
        "direction" : "out"
    }]
};

var __tars_AdminReg$checkTicket$IE = function (ticket) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, ticket);
    return os.getBinBuffer();
};

var __tars_AdminReg$checkTicket$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "uid" : is.readString(2, true, "")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$checkTicket$PE = function (ticket, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("ticket", ticket);
    return tup;
};

var __tars_AdminReg$checkTicket$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "uid" : tup.readString("uid")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$checkTicket$ER = function (data) {
    throw _makeError(data, "Call AdminReg::checkTicket failed");
};

tars.AdminRegProxy.prototype.checkTicket = function (ticket) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("checkTicket", __tars_AdminReg$checkTicket$PE(ticket, version), arguments[arguments.length - 1], __tars_AdminReg$checkTicket$IF).then(__tars_AdminReg$checkTicket$PD, __tars_AdminReg$checkTicket$ER);
    } else {
        return this._worker.tars_invoke("checkTicket", __tars_AdminReg$checkTicket$IE(ticket), arguments[arguments.length - 1], __tars_AdminReg$checkTicket$IF).then(__tars_AdminReg$checkTicket$ID, __tars_AdminReg$checkTicket$ER);
    }
};
tars.AdminRegProxy.checkTicket = __tars_AdminReg$checkTicket$IF;

var __tars_AdminReg$hasAdminAuth$IF = {
    "name" : "hasAdminAuth",
    "return" : "int32",
    "arguments" : [{
        "name" : "uid",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "has",
        "class" : "bool",
        "direction" : "out"
    }]
};

var __tars_AdminReg$hasAdminAuth$IE = function (uid) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, uid);
    return os.getBinBuffer();
};

var __tars_AdminReg$hasAdminAuth$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "has" : is.readBoolean(2, true, true)
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$hasAdminAuth$PE = function (uid, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("uid", uid);
    return tup;
};

var __tars_AdminReg$hasAdminAuth$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "has" : tup.readBoolean("has")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$hasAdminAuth$ER = function (data) {
    throw _makeError(data, "Call AdminReg::hasAdminAuth failed");
};

tars.AdminRegProxy.prototype.hasAdminAuth = function (uid) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("hasAdminAuth", __tars_AdminReg$hasAdminAuth$PE(uid, version), arguments[arguments.length - 1], __tars_AdminReg$hasAdminAuth$IF).then(__tars_AdminReg$hasAdminAuth$PD, __tars_AdminReg$hasAdminAuth$ER);
    } else {
        return this._worker.tars_invoke("hasAdminAuth", __tars_AdminReg$hasAdminAuth$IE(uid), arguments[arguments.length - 1], __tars_AdminReg$hasAdminAuth$IF).then(__tars_AdminReg$hasAdminAuth$ID, __tars_AdminReg$hasAdminAuth$ER);
    }
};
tars.AdminRegProxy.hasAdminAuth = __tars_AdminReg$hasAdminAuth$IF;

var __tars_AdminReg$hasDevAuth$IF = {
    "name" : "hasDevAuth",
    "return" : "int32",
    "arguments" : [{
        "name" : "application",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "serverName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "uid",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "has",
        "class" : "bool",
        "direction" : "out"
    }]
};

var __tars_AdminReg$hasDevAuth$IE = function (application, serverName, uid) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, application);
    os.writeString(2, serverName);
    os.writeString(3, uid);
    return os.getBinBuffer();
};

var __tars_AdminReg$hasDevAuth$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "has" : is.readBoolean(4, true, true)
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$hasDevAuth$PE = function (application, serverName, uid, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("application", application);
    tup.writeString("serverName", serverName);
    tup.writeString("uid", uid);
    return tup;
};

var __tars_AdminReg$hasDevAuth$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "has" : tup.readBoolean("has")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$hasDevAuth$ER = function (data) {
    throw _makeError(data, "Call AdminReg::hasDevAuth failed");
};

tars.AdminRegProxy.prototype.hasDevAuth = function (application, serverName, uid) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("hasDevAuth", __tars_AdminReg$hasDevAuth$PE(application, serverName, uid, version), arguments[arguments.length - 1], __tars_AdminReg$hasDevAuth$IF).then(__tars_AdminReg$hasDevAuth$PD, __tars_AdminReg$hasDevAuth$ER);
    } else {
        return this._worker.tars_invoke("hasDevAuth", __tars_AdminReg$hasDevAuth$IE(application, serverName, uid), arguments[arguments.length - 1], __tars_AdminReg$hasDevAuth$IF).then(__tars_AdminReg$hasDevAuth$ID, __tars_AdminReg$hasDevAuth$ER);
    }
};
tars.AdminRegProxy.hasDevAuth = __tars_AdminReg$hasDevAuth$IF;

var __tars_AdminReg$hasOpeAuth$IF = {
    "name" : "hasOpeAuth",
    "return" : "int32",
    "arguments" : [{
        "name" : "application",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "serverName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "uid",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "has",
        "class" : "bool",
        "direction" : "out"
    }]
};

var __tars_AdminReg$hasOpeAuth$IE = function (application, serverName, uid) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, application);
    os.writeString(2, serverName);
    os.writeString(3, uid);
    return os.getBinBuffer();
};

var __tars_AdminReg$hasOpeAuth$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "has" : is.readBoolean(4, true, true)
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$hasOpeAuth$PE = function (application, serverName, uid, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("application", application);
    tup.writeString("serverName", serverName);
    tup.writeString("uid", uid);
    return tup;
};

var __tars_AdminReg$hasOpeAuth$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "has" : tup.readBoolean("has")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$hasOpeAuth$ER = function (data) {
    throw _makeError(data, "Call AdminReg::hasOpeAuth failed");
};

tars.AdminRegProxy.prototype.hasOpeAuth = function (application, serverName, uid) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("hasOpeAuth", __tars_AdminReg$hasOpeAuth$PE(application, serverName, uid, version), arguments[arguments.length - 1], __tars_AdminReg$hasOpeAuth$IF).then(__tars_AdminReg$hasOpeAuth$PD, __tars_AdminReg$hasOpeAuth$ER);
    } else {
        return this._worker.tars_invoke("hasOpeAuth", __tars_AdminReg$hasOpeAuth$IE(application, serverName, uid), arguments[arguments.length - 1], __tars_AdminReg$hasOpeAuth$IF).then(__tars_AdminReg$hasOpeAuth$ID, __tars_AdminReg$hasOpeAuth$ER);
    }
};
tars.AdminRegProxy.hasOpeAuth = __tars_AdminReg$hasOpeAuth$IF;

var __tars_AdminReg$notifyServer$IF = {
    "name" : "notifyServer",
    "return" : "int32",
    "arguments" : [{
        "name" : "application",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "serverName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "nodeName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "command",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "result",
        "class" : "string",
        "direction" : "out"
    }]
};

var __tars_AdminReg$notifyServer$IE = function (application, serverName, nodeName, command) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, application);
    os.writeString(2, serverName);
    os.writeString(3, nodeName);
    os.writeString(4, command);
    return os.getBinBuffer();
};

var __tars_AdminReg$notifyServer$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "result" : is.readString(5, true, "")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$notifyServer$PE = function (application, serverName, nodeName, command, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("application", application);
    tup.writeString("serverName", serverName);
    tup.writeString("nodeName", nodeName);
    tup.writeString("command", command);
    return tup;
};

var __tars_AdminReg$notifyServer$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "result" : tup.readString("result")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$notifyServer$ER = function (data) {
    throw _makeError(data, "Call AdminReg::notifyServer failed");
};

tars.AdminRegProxy.prototype.notifyServer = function (application, serverName, nodeName, command) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("notifyServer", __tars_AdminReg$notifyServer$PE(application, serverName, nodeName, command, version), arguments[arguments.length - 1], __tars_AdminReg$notifyServer$IF).then(__tars_AdminReg$notifyServer$PD, __tars_AdminReg$notifyServer$ER);
    } else {
        return this._worker.tars_invoke("notifyServer", __tars_AdminReg$notifyServer$IE(application, serverName, nodeName, command), arguments[arguments.length - 1], __tars_AdminReg$notifyServer$IF).then(__tars_AdminReg$notifyServer$ID, __tars_AdminReg$notifyServer$ER);
    }
};
tars.AdminRegProxy.notifyServer = __tars_AdminReg$notifyServer$IF;

var __tars_AdminReg$pingNode$IF = {
    "name" : "pingNode",
    "return" : "bool",
    "arguments" : [{
        "name" : "name",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "result",
        "class" : "string",
        "direction" : "out"
    }]
};

var __tars_AdminReg$pingNode$IE = function (name) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, name);
    return os.getBinBuffer();
};

var __tars_AdminReg$pingNode$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readBoolean(0, true, true),
                "arguments" : {
                    "result" : is.readString(2, true, "")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$pingNode$PE = function (name, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("name", name);
    return tup;
};

var __tars_AdminReg$pingNode$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readBoolean("", false),
                "arguments" : {
                    "result" : tup.readString("result")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$pingNode$ER = function (data) {
    throw _makeError(data, "Call AdminReg::pingNode failed");
};

tars.AdminRegProxy.prototype.pingNode = function (name) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("pingNode", __tars_AdminReg$pingNode$PE(name, version), arguments[arguments.length - 1], __tars_AdminReg$pingNode$IF).then(__tars_AdminReg$pingNode$PD, __tars_AdminReg$pingNode$ER);
    } else {
        return this._worker.tars_invoke("pingNode", __tars_AdminReg$pingNode$IE(name), arguments[arguments.length - 1], __tars_AdminReg$pingNode$IF).then(__tars_AdminReg$pingNode$ID, __tars_AdminReg$pingNode$ER);
    }
};
tars.AdminRegProxy.pingNode = __tars_AdminReg$pingNode$IF;

var __tars_AdminReg$registerPlugin$IF = {
    "name" : "registerPlugin",
    "return" : "int32",
    "arguments" : [{
        "name" : "conf",
        "class" : "tars.PluginConf",
        "direction" : "in"
    }]
};

var __tars_AdminReg$registerPlugin$IE = function (conf) {
    var os = new TarsStream.TarsOutputStream();
    os.writeStruct(1, conf);
    return os.getBinBuffer();
};

var __tars_AdminReg$registerPlugin$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0)
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$registerPlugin$PE = function (conf, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeStruct("conf", conf);
    return tup;
};

var __tars_AdminReg$registerPlugin$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0)
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$registerPlugin$ER = function (data) {
    throw _makeError(data, "Call AdminReg::registerPlugin failed");
};

tars.AdminRegProxy.prototype.registerPlugin = function (conf) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("registerPlugin", __tars_AdminReg$registerPlugin$PE(conf, version), arguments[arguments.length - 1], __tars_AdminReg$registerPlugin$IF).then(__tars_AdminReg$registerPlugin$PD, __tars_AdminReg$registerPlugin$ER);
    } else {
        return this._worker.tars_invoke("registerPlugin", __tars_AdminReg$registerPlugin$IE(conf), arguments[arguments.length - 1], __tars_AdminReg$registerPlugin$IF).then(__tars_AdminReg$registerPlugin$ID, __tars_AdminReg$registerPlugin$ER);
    }
};
tars.AdminRegProxy.registerPlugin = __tars_AdminReg$registerPlugin$IF;

var __tars_AdminReg$restartServer$IF = {
    "name" : "restartServer",
    "return" : "int32",
    "arguments" : [{
        "name" : "application",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "serverName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "nodeName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "result",
        "class" : "string",
        "direction" : "out"
    }]
};

var __tars_AdminReg$restartServer$IE = function (application, serverName, nodeName) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, application);
    os.writeString(2, serverName);
    os.writeString(3, nodeName);
    return os.getBinBuffer();
};

var __tars_AdminReg$restartServer$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "result" : is.readString(4, true, "")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$restartServer$PE = function (application, serverName, nodeName, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("application", application);
    tup.writeString("serverName", serverName);
    tup.writeString("nodeName", nodeName);
    return tup;
};

var __tars_AdminReg$restartServer$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "result" : tup.readString("result")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$restartServer$ER = function (data) {
    throw _makeError(data, "Call AdminReg::restartServer failed");
};

tars.AdminRegProxy.prototype.restartServer = function (application, serverName, nodeName) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("restartServer", __tars_AdminReg$restartServer$PE(application, serverName, nodeName, version), arguments[arguments.length - 1], __tars_AdminReg$restartServer$IF).then(__tars_AdminReg$restartServer$PD, __tars_AdminReg$restartServer$ER);
    } else {
        return this._worker.tars_invoke("restartServer", __tars_AdminReg$restartServer$IE(application, serverName, nodeName), arguments[arguments.length - 1], __tars_AdminReg$restartServer$IF).then(__tars_AdminReg$restartServer$ID, __tars_AdminReg$restartServer$ER);
    }
};
tars.AdminRegProxy.restartServer = __tars_AdminReg$restartServer$IF;

var __tars_AdminReg$startServer$IF = {
    "name" : "startServer",
    "return" : "int32",
    "arguments" : [{
        "name" : "application",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "serverName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "nodeName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "result",
        "class" : "string",
        "direction" : "out"
    }]
};

var __tars_AdminReg$startServer$IE = function (application, serverName, nodeName) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, application);
    os.writeString(2, serverName);
    os.writeString(3, nodeName);
    return os.getBinBuffer();
};

var __tars_AdminReg$startServer$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "result" : is.readString(4, true, "")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$startServer$PE = function (application, serverName, nodeName, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("application", application);
    tup.writeString("serverName", serverName);
    tup.writeString("nodeName", nodeName);
    return tup;
};

var __tars_AdminReg$startServer$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "result" : tup.readString("result")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$startServer$ER = function (data) {
    throw _makeError(data, "Call AdminReg::startServer failed");
};

tars.AdminRegProxy.prototype.startServer = function (application, serverName, nodeName) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("startServer", __tars_AdminReg$startServer$PE(application, serverName, nodeName, version), arguments[arguments.length - 1], __tars_AdminReg$startServer$IF).then(__tars_AdminReg$startServer$PD, __tars_AdminReg$startServer$ER);
    } else {
        return this._worker.tars_invoke("startServer", __tars_AdminReg$startServer$IE(application, serverName, nodeName), arguments[arguments.length - 1], __tars_AdminReg$startServer$IF).then(__tars_AdminReg$startServer$ID, __tars_AdminReg$startServer$ER);
    }
};
tars.AdminRegProxy.startServer = __tars_AdminReg$startServer$IF;

var __tars_AdminReg$stopServer$IF = {
    "name" : "stopServer",
    "return" : "int32",
    "arguments" : [{
        "name" : "application",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "serverName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "nodeName",
        "class" : "string",
        "direction" : "in"
    }, {
        "name" : "result",
        "class" : "string",
        "direction" : "out"
    }]
};

var __tars_AdminReg$stopServer$IE = function (application, serverName, nodeName) {
    var os = new TarsStream.TarsOutputStream();
    os.writeString(1, application);
    os.writeString(2, serverName);
    os.writeString(3, nodeName);
    return os.getBinBuffer();
};

var __tars_AdminReg$stopServer$ID = function (data) {
    try {
        var is = new TarsStream.TarsInputStream(data.response.sBuffer);
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : is.readInt32(0, true, 0),
                "arguments" : {
                    "result" : is.readString(4, true, "")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$stopServer$PE = function (application, serverName, nodeName, __$PROTOCOL$VERSION) {
    var tup = new TarsStream.UniAttribute();
    tup.tupVersion = __$PROTOCOL$VERSION;
    tup.writeString("application", application);
    tup.writeString("serverName", serverName);
    tup.writeString("nodeName", nodeName);
    return tup;
};

var __tars_AdminReg$stopServer$PD = function (data) {
    try {
        var tup = data.response.tup;
        return {
            "request" : data.request,
            "response" : {
                "costtime" : data.request.costtime,
                "return" : tup.readInt32("", 0),
                "arguments" : {
                    "result" : tup.readString("result")
                }
            }
        };
    } catch (e) {
        throw _makeError(data, e.message, TarsError.CLIENT.DECODE_ERROR);
    }
};

var __tars_AdminReg$stopServer$ER = function (data) {
    throw _makeError(data, "Call AdminReg::stopServer failed");
};

tars.AdminRegProxy.prototype.stopServer = function (application, serverName, nodeName) {
    var version = this._worker.version;
    if (version === TarsStream.Tup.TUP_SIMPLE || version === TarsStream.Tup.TUP_COMPLEX) {
        return this._worker.tup_invoke("stopServer", __tars_AdminReg$stopServer$PE(application, serverName, nodeName, version), arguments[arguments.length - 1], __tars_AdminReg$stopServer$IF).then(__tars_AdminReg$stopServer$PD, __tars_AdminReg$stopServer$ER);
    } else {
        return this._worker.tars_invoke("stopServer", __tars_AdminReg$stopServer$IE(application, serverName, nodeName), arguments[arguments.length - 1], __tars_AdminReg$stopServer$IF).then(__tars_AdminReg$stopServer$ID, __tars_AdminReg$stopServer$ER);
    }
};
tars.AdminRegProxy.stopServer = __tars_AdminReg$stopServer$IF;



