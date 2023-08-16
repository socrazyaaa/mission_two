#include <stdio.h>
#include <string.h>
//#include <fcntl.h>        //close()
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
    int client_socket = accept(serv_socket,(struct sockaddr*) &client_addr,&addrlen);
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
