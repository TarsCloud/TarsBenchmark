- [中文文档](README.md)

## Introduction

Tarsbenchamrk system is a set of stress testing system developed based on the tar framework. It can easily complete stress testing for tars services and other services. It consists of three services:

- Benchmark/AdminServer: a stress test management service that collects data during stress tests and can only be deployed on one node
- Benchmark/NodeServer: pressure test driven service. If you need to increase the test pressure, you can deploy it on multiple nodes
- Benchmark/BenchmarkWebServer: stress test system management platform, which belongs to the extended service of tarsweb. Note that only >= (>= tarscloud/framework:v3.0.10) can be used

At the same time, the whole system depends on the database. When BenchmarkWebServer starts, it needs to connect to MySQL, and automatically creates tables. During operation, it will write data to DB

## Support description

Before < tarsweb:v3.1.0, the pressure test management platform (webserver) was built into tarsweb. In later versions, in order to provide the extensibility of tarsweb, tarsweb supports service plug-in, that is, you can realize the integration of independent web services and tarsweb, so there is no need to upgrade tarsweb when each sub module is upgraded. For specific methods, please refer to the relevant tarsweb documents

## Configuration description

When installing the stress test system, you need to rely on MySQL. Therefore, pay attention to configuring the dependent MySQL address during installation

- Webserver please modify `config json`

## Pressure measuring tool description

You can compile based on the source code to generate an independent stress testing tool (TB), which can be used to complete stress testing (command line interaction mode) [please refer to the documentation](../docs/benchmark.en.md)
