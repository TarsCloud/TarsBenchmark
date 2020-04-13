[点我查看中文版](README.md)

## Introduction

tb is an uncoded pressure measurement tool specially tailored for tars services. It utilizes event-driven and multi-process to fully explore the performance of the pressure measurement machine and has the following features:

 - High network performance(8-core machine supports more than 20W/S TPS)
 - Communication scalability(The network layer supports two TCP / UDP protocol modes)
 - Protocol scalability(the application layer supports Http/Tars service pressure testing)
 - Perfect real-time statistics and monitoring support. Provide the number of requests/TPS/time-consuming/success rate distribution in the cycle

## How to compile tb
###  Dependence
Install TarsCpp development environment before compiling tb, since it is based on [TarsCpp](https://tarscloud.gitbook.io/tarsdocs/rumen/env/tarscpp).

### Compilation:
```
mkdir build; cd build; cmake ..; make;
```

## Usage
### Example
```
./tb -c 600 -s 6000 -D 192.168.31.1 -P 10505 -p tars -S tars.DemoServer.DemoObj -M test -C test.txt
```

### Case File generation
The use case file is recommended to be automatically generated using the tars2case tool. Users can modify the parameter values according to business needs
```
/usr/local/tars/cpp/tools/tars2case Demo.tars --dir=benchmark

cd benchmark && ls
echo.case  test.case
```

### Introduction to writing case
The file is divided into upper and lower parts, separated by a line beginning with "#", the upper part is the RPC parameter, and the lower part is the value of the RPC call parameter, which corresponds to the parameter one by one

- **Parameter help description**：
1. Input parameters are separated by "|" symbols, that is, "," in tars parameters are replaced with "|"
2. Parameters of struct like: struct <tag require | optional field 1, field 2, field 3 ...>, if tag starts from 0, directly field 1
3. Parameters of vector like: vector <type>
4. Parameters of map lik: map <key type, value type>
5. 2, 3, 4 can be nested

- **Parameter Value help description**：
1. <strong> Basic type </strong> random value setting:
   <strong>Random Random Value</strong> like as [1-100], which means it appears randomly within 1-100, type must be a number
   <strong>Limited random value</strong> like as [1,123,100], which means it appears randomly in 1,123,100, which type can be a string
2. Enter the parameters for each parameter line, that is, the "," in the tars parameter list is replaced with a carriage return
3. Value of struct like: <field value 1, field value 2, field value 3 ...>
4. Value of vector like: <value 1, value 2, value 3 ...>
5. Value of map like: [key1 = val1, key2 = val2, key3 = val3 ...]
6. 3, 4, 5 can be nested

- **E.g**：
```
vector<string>|struct<string, int>|map<string, string>
#######
<abc, def, tt, fbb>
<abc, 1>
[abc=def, dfd=bbb]
```

### Pressure test result display
![results](docs/image/result.jpg)

## FAQ

 - What is the solution for the specified rate of tb
1. The specified unit of rate is a single target machine. If there are n target machines, the statistical TPS = rate * n
2. If the pressure measurement rate is not specified, tb will launch a shock to the target machine at full speed.

 - How is tb high performance achieved?
First, avoid the network IO blocking of the process through event-driven, maximize the CPU utilization,
Secondly, the tool will create the same number of pressure test processes based on the number of CPU cores, and the number of connections and the pressure test rate will be divided equally among the child processes.

 - How to synchronize data synchronization between tb pressure testing process?
Data sharing is achieved through a lock-free shared memory queue, and the statistical results are output in the main process