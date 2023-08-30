#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#define BUF_SIZE 4*1024 //每次发送4k的数据

class TcpServer;

//需要发送的数据结构体包含需要发送的套接字、消息源的ip
struct Msg{
	int sockfd;      //套接字
	char ip[15];	 // ip地址
	TcpServer* ser;
	Msg(int fd,char* str,TcpServer* ser):sockfd(fd),ser(ser){
		memset(ip,0,sizeof(ip));
		strcpy(ip,str);
	}
};

class TcpServer{
	public:
		pthread_rwlock_t sockfd_rwlock;		//读写锁:用于维护连接到服务器的客户端套接字
		int sock_arr_index;					//统计连接到服务器的客户端个数
		int* sockfd_array;					//用于保存连接的文件描述数组 
		int max_client;	    				//最大连接数

		int m_sockfd;				        //启动时的套接字
		int m_port;				            //启动监听的端口
		
		static int buf_size;				//发送字符串的大小
	public:
		//无参构造
		TcpServer();
		/*
		 * TcpServer(int port,int max_client)：有参构造，设置监听的端口和最大连接数
		 * @port：监听的端口
		 * @max_client：支持最大连接的客户端数
		 */
		TcpServer(int port,int max_client);
		~TcpServer();

		/*
		 *BlindAndListen - 绑定ip和端口，并进入监听状态：socket()->blind()->listen()
		 */
		void BlindAndListen();

		/*
		 *ConnectToClient - 用于循环监听,返回监听到的套接字: accept() ; 返回监听到的套接字，并使用client_ip记录源IP
		 */
		bool ConnectToClient(char* client_ip,int* client_sockfd);

		/*
		 *Broadcast - 向套接字数组sockfd_array中的每一个套接字发送buf信息
		 *@sockfd : 不向此套接字发送信息。用于避免回声的现象，sockfd = 0，表示转发到所有的套接字。
		 *@buf  ：需要发送的字符串
		 *@buf_size：字符串长度
		 */
		static void Broadcast(int sockfd,char* buf, int buf_size,TcpServer* ser);

		/*
		 * ReadAndBroadcast - 读取套接字的消息，输入并将其转发到其余的客户端
		 * @arg：Msg 结构体，保存 源信息的ip 和 套接字sockfd 和 TcpServer对象指针
		 */
		static void* ReadAndBroadcast(void* argv);

		/*
		 *AddSockfd - 向套接字数组中添加客户端套接字。客户端计数 sock_arr_index 加 1
		 *@sockfd:需要插入的socket套接字
		 */
		void AddSockfd(int sockfd);

		/*
		 *DeleteSockfd - 在套接字数组中删除指定套接字。当客户端断开连接后，及时关闭socket套接字。
		 *@sockfd：需要关闭的套接字
		 */
		static void DeleteSockfd(int sockfd,TcpServer* ser);

		/*
		*SaveFileformClient - 接受套接字发送的文件
		*@sockfd：套接字信息
		*/
		void SaveFileformClient(int sockfd);

		/*
		*getSaveFileName - 找到一个不存在的文件名字,将文件保存在./download 文件夹下
		*@file_name:文件名字
		*/
		void getSaveFileName(char* file_name);
};


#endif //_SERVER_H

