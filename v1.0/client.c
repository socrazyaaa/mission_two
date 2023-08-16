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
