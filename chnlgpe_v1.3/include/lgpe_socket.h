/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe
文 件 名 : lgpe_socket.h
创建日期 : 2014年3月12日
文件说明 : 

修改历史 :
REV1.0.0  hongliang  2014年3月12日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe
File Name    : lgpe_socket.h
Create Date  : 2014年3月12日
Description  : 

Modification History
REV1.0.0  hongliang  2014年3月12日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _SOCKET_H_
#define _SOCKET_H_

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*----------------------------------------------------------*
 * 调试开关             Debug Switch Section                *
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件             Include File Section                *
 *----------------------------------------------------------*/

//#include "lgpe_header.h"

/*---------------------------------------------------------*
 * 全局宏定义           Global Macro Define Section         *
 *----------------------------------------------------------*/

#define SOCKET_CMD_PORT

/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     *
 *----------------------------------------------------------*/

typedef struct _connelem {
	int conn_fd;
	void* next;
	/** Receive state */
	struct {
		unsigned char buffer[BUFSIZE];
		int bufpos, bufused;
		unsigned char packet[BUFSIZE];
		int count;
	} recv;
} LGPEConnElem;

typedef struct _connlist {
  int length;
  int server_fd;
  LGPEConnElem* head;
  LGPEConnElem* tail;
} LGPEConnList;
typedef struct _conlistlist {
	int length;
	LGPEConnList* head;
	LGPEConnList* tail;
} LGPEconnListList;

/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     *
 *----------------------------------------------------------*/

extern volatile int g_dsplinkExit;

LGPEConnList*  g_server_clients_data;
#ifdef SOCKET_CMD_PORT
LGPEConnList*  g_server_clients_cmd;
#endif
/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    *
 *----------------------------------------------------------*/

/*socket初始化函数*/
void lgpe_Socket_Initialize();
void lgpe_Socket_Finalize();
void *lgpe_Socket_Recv_Thread();

void fd_wait(fd_set *fds, int *maxfd, int fd);
/*socket检测客户端事件
 * clist：连接链表
 * fds：fd集合
 * porttype：socket端口类型 0：data端口10002   1：cmd端口 10003
 * */
void event_check_clients(LGPEConnList* clist, fd_set* fds, int porttype);
void lgpe_Socket_Handle_Server_In(int fd,LGPEConnList *clist);
LGPEConnElem* lgpe_Socket_Create_Elem(int cfd);
void lgpe_Socket_Add_Client(LGPEConnList* clist, LGPEConnElem* elem);
void event_add_clients(LGPEConnList* clist, fd_set* fds, int* maxfd);
/*socket客户端消息处理函数
 * clist：连接链表
 * connfd：连接fd
 * porttype：socket端口类型 0：data端口10002   1：cmd端口 10003
 * */
void lgpe_Socket_Handle_Client_In(LGPEConnList* clist,int connfd,int porttype);
LGPEConnElem* get_elem_by_fd(LGPEConnList* clist,int sfd);
int sock_read_byte(LGPEConnElem* elem);
int sock_read_and_process(LGPEConnElem* elem);
//void static lgpe_Socket_Free_Elem(LGPEConnElem* elem);
void lgpe_Socket_Remove_Client(LGPEConnList* clist, int fd);
void lgpe_Socket_Dispatch_To_Clients(LGPEConnList* clist, unsigned char* packet, int len);
int safewrite(int fd, const void *buffer, int count);
LGPEConnList* lgpe_Socket_Create_List(int sfd);
int lgpe_Socket_Open_Server_Port(int port);
void lgpe_Socket_Remove_List(LGPEConnList* clist);

/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   *
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _SOCKET_H_ */
