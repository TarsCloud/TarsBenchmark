[点我查看中文版](README.zh.md)

## Introduction

**tb**(TarsBenchmark)It is a non-code benchmark tool specially tailored for tars service, with the following features:

 - High performance: 8-core machine TPS supports more than 20W/s;
 - Network compatibility: The network layer supports TCP and UDP protocol;
 - Protocol scalability: It supports http/tars service benchmark, open to third-protocol agreements
 - Perfect real-time monitor. Provide the number of TPS/Success Rate/Cost time within the cycle;

## Framework

The tb is designed in a multi-process model. The main process is responsible for resource scheduling and display, and the benchmark process is responsible for network transmission and reception and statistics. The network layer can flexibly choose TCP or UDP; adopts a protocol factory to manage various service protocols, supports http/tars by default; the main process and the benchmark process exchange signals through control information, and the data interacts through the lock-free shared memory queue to achieve the lowest resource consumption. The main process periodically collects each The network statistics of the benchmark process are output to the console after a simple summary.

![tb system](https://github.com/TarsCloud/TarsDocs_en/blob/master/assets/tb-platform.png)


## Usage

Sample
```text
./tb -c 600 -s 6000 -D 192.168.31.1 -P 10505 -p tars -S tars.DemoServer.DemoObj -M test -C test.txt
```
Description
```text
  -h                   help information
  -c                   number of connections
  -D                   target server address(ipv4)
  -P                   target server port
  -p                   service protocol(tars|http)
  -t(optional)         overtime time，default 3 second
  -T(optional)         network protocol，default tcp
  -I(optional)         continue time(by second)，default 1h
  -i(optional)         view interval(by second)，default 10s
  -s(optional)         maximum tps limit per target server
  -n(optional)         maximum process
```
See details in [develop.md](https://github.com/TarsCloud/TarsDocs_en/blob/master/benchmark/develop.md)

## Quick Start

The online benchmark service can be implemented with the latest version of [TarsWeb](https://github.com/TarsCloud/TarsWeb). release steps are as follows:

```shell
./install.sh webhost token adminsip nodeip
```

Description:
```text
webhost                  Host or ip:port on the TarsWeb management side
token                    Which can obtain the http://webhost/auth.html#/token through the management side
adminsip                 The IP address of the AdminServer deployment, it must be deployed at a single point
nodeip                   The IP address of the NodeServer deployment, it should be separated from the AdminServer
```
**AdminServer** is recommended to deploy together with tarsregistry, **NodeServer** is recommended to expand the capacity on the management side. The more machines deployed, the stronger the ability to support parallel benchmark.
