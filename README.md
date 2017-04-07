# Project Description
This project is to implement a simple traffic generator with EPOLL. It consists of two components: a client and a server. The client establishs a number of TCP flows with the server. And then a fixed number of connections send requests to the server, requiring responces with specific sizes. After receiving the responce, the flow will be closed by the client. The generator ensures that the number of concurrent flows transmitting data is fixed.

# Folders Description
## conf
The flow size distribution setting flie cdf is in it. I will give an example to explain it.<br/>
10000 0 <br/>
100000 0.7 <br/>
1000000 1 <br/>
It denotes that the proportion of flows whose flow sizes are less than 10000 is 0, less than 100000 is 0.7 and less than 1000000 is 1.
## src
The source code folder
## result
The files in result/ are about the raw data recording the flow size and the flow complete time (FCT). For example, <br/>
435224 3452 <br/>
means the complete time of the flow whose size is 435224 Bytes is 3452us.
## stats
The files in stats/ are about the processed data: the 50th percentage FCT and the 99th percentage.
The filename of files in result/ and stats/ can be modified in src/client.h

# How to use

## Compiling
$ make <br/>
you will find two executable files client and server in bin/<br/>

## Quick start
### Server
In the main directory, <br/>
$ ./bin/server -p 9001 -t 12
### Client
In the main directory, <br/>
$ ./bin/client -n 500 -c 10 -p 9001 -t 12 -i eth2 -s 172.16.32.207<br/>

## Arguments Description
### Server
-p: server port<br/>
-t: the number of threads to process packets (send)<br/>
-h: display help information<br/>
### Client
-n: the total number of flows to establish<br/>
-c: the number of concurrency<br/>
-p: server port<br/>
-t: the number of threads to process packets (receive)<br/>
-i: the network interface<br/>
-s: server ip<br/>
