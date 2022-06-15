- [English](Benchmark.en.md)

## 简介

TarsBenchamrk 系统是基于 tars 框架开发的一套压力测试系统, 可以方便的对 Tars 服务以及其他服务完成压力测试, 它有三个服务组成:

- benchmark/AdminServer: 压力测试管理服务, 收集压力测试过程中的数据, 只能部署在一台节点上
- benchmark/NodeServer: 压测试驱动服务, 如果需要加大测试压力, 可以部署多节点上
- benchmark/WebServer: 压力测试系统管理平台, 它属于 TarsWeb 的扩展服务, 注意 >= TarsFramework:v3.1.0 & TarsWeb:v3.1.0 才可以使用

同时整个系统依赖数据库, WebServer 启动时需要连接 mysql, 且自动会创建表, 运行过程中会将数据写入到 db 中.

## 支持说明

在< TarsWeb:v3.1.0 之前, 压测管理平台(WebServer)被内置在 TarsWeb 中, 之后版本为了提供 TarsWeb 的扩展性, TarsWeb 支持了服务插件化, 即你可以实现独立的 web 服务和 TarsWeb 整合到一起, 从而当各个子模块升级时无须升级 TarsWeb, 具体方式请参考 TarsWeb 相关的文档.

## 安装方式

推荐使用新版本 > TarsFramework:v3.1.0 时, 直接从云市场安装网关服务, 建议以容器方式启动网关, 这样不依赖操作系统 stdc++.so 的版本.

[容器方式启动业务方式请参考](https://doc.tarsyun.com/#/installation/service-docker.md)

## 配置说明

在安装压力测试系统时, 需要依赖 mysql, 因此在安装注意配置依赖的 mysql 地址

- WebServer 请修改`config.json`

## 压测工具说明

你可以基于源码编译, 生成独立的压力测试工具(tb), 可以使用它来完成压力测试(命令行交互模式), [请参考说明文档](./Benchmark.md)
