# Client - Server application in C++ using Berkeley Sockets

## How to run server
1. build project;
2. ./server \<protocol\> -p \<port\>, 
where \<protocol\> either "-t" (TCP) or "-u" (UDP), \<port\> server's port

## How to run client
1. build project;
2. ./client \<protocol\> -a <server_addr> -p \<server_port\>, 
where \<protocol\> either "-t" (TCP) or "-u" (UDP), \<server_port\> server's port, \<server_addr\> - server's address

## Example
./server -t -p 1234

./client -t -a localhost -p 1234
