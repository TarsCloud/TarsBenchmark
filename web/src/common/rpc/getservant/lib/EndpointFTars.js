// **********************************************************************
// Parsed By TarsParser(2.4.20), Generated By tools(20200627)
// TarsParser Maintained By <TARS> and tools Maintained By <superzheng>
// Generated from "EndpointF.tars" by Structure Mode
// **********************************************************************

/* eslint-disable */

"use strict";

var TarsStream = require("@tars/stream");

var _hasOwnProperty = Object.prototype.hasOwnProperty;

var tars = tars || {};
module.exports.tars = tars;

tars.EndpointF = function() {
    this.host = "";
    this.port = 0;
    this.timeout = 0;
    this.istcp = 0;
    this.groupworkid = 0;
    this.grouprealid = 0;
    this.setId = "";
    this.bakFlag = 0;
    this._classname = "tars.EndpointF";
};
tars.EndpointF._classname = "tars.EndpointF";
tars.EndpointF._write = function (os, tag, value) { os.writeStruct(tag, value); };
tars.EndpointF._read  = function (is, tag, def) { return is.readStruct(tag, true, def); };
tars.EndpointF._readFrom = function (is) {
    var tmp = new tars.EndpointF;
    tmp.host = is.readString(0, true, "");
    tmp.port = is.readInt32(1, true, 0);
    tmp.timeout = is.readInt32(2, true, 0);
    tmp.istcp = is.readInt32(3, true, 0);
    tmp.groupworkid = is.readInt32(5, false, 0);
    tmp.grouprealid = is.readInt32(6, false, 0);
    tmp.setId = is.readString(7, false, "");
    tmp.bakFlag = is.readInt32(9, false, 0);
    return tmp;
};
tars.EndpointF.prototype._writeTo = function (os) {
    os.writeString(0, this.host);
    os.writeInt32(1, this.port);
    os.writeInt32(2, this.timeout);
    os.writeInt32(3, this.istcp);
    os.writeInt32(5, this.groupworkid);
    os.writeInt32(6, this.grouprealid);
    os.writeString(7, this.setId);
    os.writeInt32(9, this.bakFlag);
};
tars.EndpointF.prototype._equal = function (anItem) {
    return this.host === anItem.host && 
        this.port === anItem.port && 
        this.timeout === anItem.timeout && 
        this.istcp === anItem.istcp;
};
tars.EndpointF.prototype._genKey = function () {
    if (!this._proto_struct_name_) {
        this._proto_struct_name_ = "STRUCT" + Math.random();
    }
    return this._proto_struct_name_;
};
tars.EndpointF.prototype.toObject = function() { 
    return {
        "host" : this.host,
        "port" : this.port,
        "timeout" : this.timeout,
        "istcp" : this.istcp,
        "groupworkid" : this.groupworkid,
        "grouprealid" : this.grouprealid,
        "setId" : this.setId,
        "bakFlag" : this.bakFlag
    };
};
tars.EndpointF.prototype.readFromObject = function(json) { 
    _hasOwnProperty.call(json, "host") && (this.host = json.host);
    _hasOwnProperty.call(json, "port") && (this.port = json.port);
    _hasOwnProperty.call(json, "timeout") && (this.timeout = json.timeout);
    _hasOwnProperty.call(json, "istcp") && (this.istcp = json.istcp);
    _hasOwnProperty.call(json, "groupworkid") && (this.groupworkid = json.groupworkid);
    _hasOwnProperty.call(json, "grouprealid") && (this.grouprealid = json.grouprealid);
    _hasOwnProperty.call(json, "setId") && (this.setId = json.setId);
    _hasOwnProperty.call(json, "bakFlag") && (this.bakFlag = json.bakFlag);
    return this;
};
tars.EndpointF.prototype.toBinBuffer = function () {
    var os = new TarsStream.TarsOutputStream();
    this._writeTo(os);
    return os.getBinBuffer();
};
tars.EndpointF.new = function () {
    return new tars.EndpointF();
};
tars.EndpointF.create = function (is) {
    return tars.EndpointF._readFrom(is);
};

