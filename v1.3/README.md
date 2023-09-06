# socket双工通信+文件传输

在此项目中，每一个节点既拥有客户端的功能（向指定ip发送信息），也用有服务器的功能（接收连接该服务端的客户端的信息，并将其转发给连接到该服务端的其余客户端).并支持上传文件到服务器。  
v1.3更新如下
- 支持向客户端向服务端上传文件。服务器端将客户端上传的文件保存到`./download/`目录下;
- 支持仅向服务器或者连接到此电脑的客户端发送消息。
# 文件结构

```
.
├── client.cpp
├── client.h
├── server.cpp
├── server.h
├── README.md
├── main.cpp
└── util.h

0 directories, 7 files
```
# 运行
`客户端`
```shell
$ ./app xx.xx.xx.131
请按以下格式输入需要执行的任务
@server:xxxxx   @client:xxxxx   @fileTransfer:xxxxx
@fileTransfer:./app
The file information has been sent successfully.File transfer start...
send 4096 bytes,total send 4096/33304 bytes
send 4096 bytes,total send 8192/33304 bytes
send 4096 bytes,total send 12288/33304 bytes
send 4096 bytes,total send 16384/33304 bytes
send 4096 bytes,total send 20480/33304 bytes
send 4096 bytes,total send 24576/33304 bytes
send 4096 bytes,total send 28672/33304 bytes
send 4096 bytes,total send 32768/33304 bytes
send 536 bytes,total send 33304/33304 bytes
./app文件传输完成！
```

`服务器端`
```shell
$ ./app
请按以下格式输入需要执行的任务
@server:xxxxx   @client:xxxxx   @fileTransfer:xxxxx
Wed Sep  6 14:37:16 2023
---------       xx.xx.xx.59 加入了连接 -----------
start recevie file, file name:app   file size:33304
write 4096 bytes,total write 4096/33304 bytes
write 4096 bytes,total write 8192/33304 bytes
write 4096 bytes,total write 12288/33304 bytes
write 4096 bytes,total write 16384/33304 bytes
write 4096 bytes,total write 20480/33304 bytes
write 4096 bytes,total write 24576/33304 bytes
write 4096 bytes,total write 28672/33304 bytes
write 4096 bytes,total write 32768/33304 bytes
write 536 bytes,total write 33304/33304 bytes
file has been saved to ./download/app successfully

```
