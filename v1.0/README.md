# socket通信

## 一、使用socket编程实现回声客户端
所谓“回声”，是指客户端向服务器发送一条数据，服务器再将数据原样返回给客户端，就像声音一样，遇到障碍物会被“反弹回来”  
`server.c`:
```c
#include <stdio.h>
#include <string.h>
//#include <fcntl.h>       
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc,char **argv){
    //1.创建tcp套接字
    int serv_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(serv_socket < 0 ){
        printf("create socket error!\n");
        return 1;
    }

    //2. 创建sockaddr_in结构体变量
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));            //全部置0
    addr.sin_family      = AF_INET;               //IPv4
    addr.sin_port        = htons(8000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");//本地环回地址

    //3.将套接字和IP、端口绑定
    int ret = bind(serv_socket, (struct sockaddr*)&addr, sizeof(addr));
    if(ret != 0){
        printf("bind port error!\n");
        return 1;
    }

    //4.启动监听
    printf("启动监听！\n");
    ret = listen(serv_socket,10);            //最大支持10个连接
        
    //5. 接受连接
    struct sockaddr_in client_addr;         //用于记录连接服务器的客户端信息
    memset(&client_addr,0,sizeof(client_addr));
    socklen_t addrlen = sizeof(client_addr);
    int client_socket = accept(serv_socket,(struct sockaddr*)&client_addr,&addrlen);
    printf("连接成功！\n");

    //6.收发数据
    char buf[512]={0};
    int len  =read(client_socket,&buf,512); //接收数据
    printf("收到数据：%s\n",buf);
    ret      =write(client_socket,&buf,512);//发送数据

    //7. 关闭套接字
    close(serv_socket);
    close(client_socket);
    return 0; 
}
```
客户端源程序`client.c`:
```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc,char **argv){
    //1. 创建socket 套接字
    int client_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(client_socket < 0 ){
        printf("error: create client_socket failed !\n");
        return 1;
    }
    printf("create client_socket successful! \n");

    //2.设置服务器 IP和端口
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(8000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //3.连接服务器
    int ret = connect(client_socket,(struct sockaddr*)&addr,sizeof(addr));
    if(ret < 0 ){
        printf("error:connect to serve failed!\n");
        return 0;
    }
    printf("connect to serve successful!\n");

    //4. 收发数据
    char send_buf[] = "nice to see you !";
    write(client_socket,send_buf,sizeof(send_buf));
    char read_buf[512];
    memset(&read_buf,0,512);
    read(client_socket,read_buf,512);
    printf("message from serve:\n %s \n",read_buf);
    close(client_socket);
    return 0;
}

```
### 运行
创建两个终端，分别运行`server.c`和`client.c`。服务器先运行，然后开启监听：   
服务器:
```shell
$ gcc server.c -o server
$ gcc client.c -o client 
$ ./server
启动监听！
```
客户端终端：
```shell
$ ./client
create client_socket successful!
connect to serve successful!
message from serve:
 nice to see you !
```
服务器:
```shell
$ gcc server.c -o server
$ gcc client.c -o client 
$ ./server
启动监听！
连接成功！
收到数据：nice to see you !
```
## 二、相关函数

### 函数及头文件

| 头文件 | 函数 | 函数原型|
|:---|:---|:---|
|`<sys/socket.h>`|`socket()`|`int socket(int af, int type, int protocol)`|
| | `bind()`|`int bind(int sockfd, struct sockaddr *addr, socklen_t addrlen);`|
| | `listen()`|`int listen(int sockfd, int backlog);  `|
||`accept()`|`int accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen);`|
||`connect()`|`int connect(int sockfd,struct sockaddr *serv_addr,int addrlen);`|
||`struct sockaddr`||
|`<unistd.h>`|`write()`|`ssize_t write(int fd, const void *buf, size_t count);`|
||`read()`|`ssize_t read(int fd, const void *buf, size_t count);`|
|`<arpa/inet.h>`|`htons()`|`uint16_t htons(uint16_t hostshort);`|
|`<netinet/in.h`>|`struct sockaddr_in`||

### socket 函数
在`Linux`下使用 `<sys/socket.h>` 头文件中 `socket()` 函数来创建套接字，原型为：
```c
int socket(int af, int type, int protocol);
```
- `af(Address Family)`：地址族(IP 地址类型)，有IPv4(`AF_INET`) 和IPv6(`AF_INET6`)。
- `type`: 数据传输方式/套接字类型，常用的有 `SOCK_STREAM`（流格式套接字/面向连接的套接字） 和 `SOCK_DGRAM`（数据报套接字/无连接的套接字）
>根据数据的传输方式，可以将 Internet 套接字分成两种类型。通过 `socket()` 函数创建连接时，必须告诉它使用哪种数据传输方式。  
>>流格式套接字（Stream Sockets）也叫“面向连接的套接字”，在代码中使用   `SOCK_STREAM` 表示。  
>>数据报格式套接字（SOCK_DGRAM）数据报格式套接字（Datagram Sockets）也叫“无连接的套接字”，在代码中使用 `SOCK_DGRAM` 表示。
- `protocol` :数据传输协议，有TCP 传输协议(`IPPROTO_TCP`)和 UDP 传输协议(` IPPROTO_UDP`)。
```c
//一、创建tcp
int tcp_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
//二、创建udp
int udp_socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)
```
**也可以将`protocol`的值设置为0，系统自动推演该使用什么协议。**

### `blind()`和`connect()`:绑定套接字并建立连接
`socket()` 函数用来创建套接字，确定套接字的各种属性，然后服务器端要用 `bind()` 函数将套接字与特定的 `IP` 地址和端口`PORT`绑定起来，只有这样，流经该 `IP` 地址和端口的数据才能交给套接字处理。类似地，客户端也要用 `connect()` 函数建立连接。`bind()` 函数和`connect()` 函数的原型为：
```c
//server
int bind(int socket, struct sockaddr *addr, socklen_t addrlen);
//client
int connect(int socket, struct sockaddr *addr, socklen_t addrlen); 
```
- `addr` 为 `sockaddr` 结构体变量的指针。
- `addrlen` 为 `addr` 变量的大小，可由 sizeof() 计算得出。

示例：
```c
//创建套接字IPv4,TCP连接
int serv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

//创建sockaddr_in结构体变量
struct sockaddr_in serv_addr;
memset(&serv_addr, 0, sizeof(serv_addr)); //用0 填充每个字节
serv_addr.sin_family = AF_INET;  //使用IPv4地址
serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
serv_addr.sin_port = htons(1234);  //端口

//将套接字和IP、端口绑定
bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
```
`sockaddr_in` 结构体:`sockaddr` 是一种通用的结构体，可以用来保存多种类型的IP地址和端口号，而 `sockaddr_in` 是专门用来保存 IPv4 地址的结构体,而 `sockaddr_in6` 是专门用来保存 IPv6 地址的结构体。
```c
struct in_addr{
    in_addr_t  s_addr;  //32位的IP地址
};

struct sockaddr{
    sa_family_t  sin_family;   //地址族（Address Family），也就是地址类型
    char         sa_data[14];  //IP地址和端口号
};

struct sockaddr_in{
    sa_family_t     sin_family;   //地址族（Address Family），也就是地址类型
    uint16_t        sin_port;     //16位的端口号
    struct in_addr  sin_addr;     //32位IP地址
    char            sin_zero[8];  //不使用，一般用0填充
};

```
- `sin_prot` 为端口号。理论上端口号的取值范围为 `0 ~ 65536`,但 `0 ~ 1023` 的端口一般由系统分配给特定的服务程序,所以我们的程序要尽量在 `1024 ~ 65536` 之间分配端口号。**端口号需要用 `htons()` 函数转换**  

**`bind()` 函数和`connect()` 函数的第二个参数类型为 `sockaddr`，而代码中却使用 `sockaddr_in`，然后再强制转换为 `sockaddr`，这是为什么呢？**
>`sockaddr` 和 `sockaddr_in` 的长度相同，都是16字节，只是`sockaddr`将IP地址和端口号合并到一起，用一个成员 `sa_data` 表示。要想给 sa_data 赋值，必须同时指明IP地址和端口号，例如`”127.0.0.1:80“`，遗憾的是，没有相关函数将这个字符串转换成需要的形式，也就很难给 `sockaddr` 类型的变量赋值，所以使用 `sockaddr_in` 来代替。这两个结构体的长度相同，强制转换类型时不会丢失字节，也没有多余的字节。
`in_addr_t` 在头文件`<netinet/in.h>` 中定义，等价于 `unsigned long`，长度为4个字节。  

### `listen()`和`accept()`函数：让套接字进入监听状态并响应客户端请求
对于服务器端程序，使用`bind()`绑定套接字后，还需要使用`listen()`函数让套接字进入被动监听状态。当套接字处于监听状态时，再调用`accept()`函数，就可以随时响应客户端的请求了。其函数原型如下：
```c
int listen(int socket, int backlog);  
int accept(int socket, struct sockaddr *addr, socklen_t *addrlen); 
```
- `backlog` 为请求队列的最大长度。
>当套接字正在处理客户端请求时，如果有新的请求进来，套接字是没法处理的，只能把它放进缓冲区，待当前请求处理完毕后，再从缓冲区中读取出来处理。如果不断有新的请求进来，它们就按照先后顺序在缓冲区中排队，直到缓冲区满。这个缓冲区，就称为请求队列（Request Queue）。 `SOMAXCONN`则由系统来决定请求队列长度。
- 监听：是指当没有客户端请求时，套接字处于“睡眠”状态，只有当接收到客户端请求时，套接字才会被“唤醒”来响应请求。
**`accept()` 返回一个新的套接字来和客户端通信，`addr` 保存了客户端的IP地址和端口号，而 `socket` 是服务器端的套接字。后面和客户端通信时，要使用这个新生成的套接字，而不是原来服务器端的套接字。**

### `write()/read()`函数：发送与接收数据
`Linux` 不区分套接字文件和普通文件，使用 `write()` 可以向套接字中写入数据，使用 `read()` 可以从套接字中读取数据。
> 两台计算机之间的通信相当于两个套接字之间的通信，在服务器端用 `write() `向套接字写入数据，客户端就能收到，然后再使用 `read()` 从套接字中读取出来，就完成了一次通信。  
`write()/read()`函数原型如下：
```c
ssize_t write(int fd, const void *buf, size_t nbytes);
ssize_t read(int fd, const void *buf, size_t nbytes);
```
- `fd`:文件描述符
- `buf`：用于接收或者发送数据的缓冲区地址
- `nbytes`:需要写入或者读取的字节数
>`write()` 函数会将缓冲区 `buf` 中的 `nbytes` 个字节写入文件 `fd`，成功则返回写入的字节数，失败则返回 `-1` 
>`read()` 函数会从 `fd` 文件中读取 `nbytes` 个字节并保存到缓冲区 `buf`，成功则返回读取到的字节数（但遇到文件结尾则返回0），失败则返回 `-1`。



