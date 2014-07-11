/******************************************************************************
 版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
 工 程 名 : chnlgpe
 文 件 名 : socket.c
 创建日期 : 2014年3月12日
 文件说明 :

 修改历史 :
 REV1.0.0  hongliang  2014年3月12日  文件创建

 ******************************************************************************
 Copyright    : LGPE Co.,Ltd. All Rights Reserved.
 Project Name : chnlgpe
 File Name    : socket.c
 Create Date  : 2014年3月12日
 Description  :

 Modification History
 REV1.0.0  hongliang  2014年3月12日  File Create

 ******************************************************************************/

/*----------------------------------------------------------*
 * 调试开关           Debug Switch Section                  *
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件           Include File Section                  *
 *----------------------------------------------------------*/

/*  ----------Application Header-------------*/
#include "lgpe_header.h"
/*----------------------------------------------------------*
 * 全局变量定义       Global Variable Define Section        *
 *----------------------------------------------------------*/

struct sockaddr_in client_addr;
volatile int g_sockfd_data, g_sockfd_cmd/*,sockfd_original*/, new_fd;
extern pthread_mutex_t mutex;
extern pthread_mutex_t sql_mutex;
extern sem_t sem;
extern LGPEsqloperateList* g_sqlOperate_list;

/*----------------------------------------------------------*
 * 局部宏定义         Local Macro Define Section            *
 *----------------------------------------------------------*/
#define SOCKET_DATA 0
#define SOKCET_CMD 1
/*----------------------------------------------------------*
 * 局部结构定义       Local Structure Define Section        *
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 局部函数原型声明   Local Prototype Declare Section       *
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 静态变量定义       Static Variable Define Section        *
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 函数定义           Function Define Section               *
 *----------------------------------------------------------*/
//extern void sigroutine(int dunno);
void lgpe_Socket_Initialize()
{
	g_sockfd_data = lgpe_Socket_Open_Server_Port(10002); /*开启socket10002端口 用于数据接受发送*/
	g_server_clients_data = lgpe_Socket_Create_List(g_sockfd_data); /*创建连接队列*/
#ifdef SOCKET_CMD_PORT
	g_sockfd_cmd = lgpe_Socket_Open_Server_Port(10003); /*开启socket10003端口 用于命令接受发送*/
	g_server_clients_cmd = lgpe_Socket_Create_List(g_sockfd_cmd); /*创建连接队列*/
#endif
}

void lgpe_Socket_Finalize()
{
	lgpe_Socket_Remove_List(g_server_clients_data);
	close(g_sockfd_data);
#ifdef SOCKET_CMD_PORT
	lgpe_Socket_Remove_List(g_server_clients_cmd);
	close(g_sockfd_cmd);
#endif
}

/** ============================================================================
 *  @func   lgpe_Socket_Recv_Thread
 *
 *  @desc   This thread receives data from QT through socket.
 *
 *  @modif  None
 *  ============================================================================
 */
void *lgpe_Socket_Recv_Thread()
{
	fd_set skfds;
	int maxfd;
	int ret;
//	unsigned char sSocketFrame[1024];
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);/*线程可被结束*/
	/*信号拦截*/
//	signal(SIGINT, sigroutine);
//	signal(SIGTERM, sigroutine);
	for (;;)
	{
		maxfd = -1;
		FD_ZERO(&skfds);
		/*添加运行数据的sockfd_data到fdset*/
		fd_wait(&skfds, &maxfd, g_sockfd_data);
#ifdef SOCKET_CMD_PORT
		/*添加运行数据的sockfd_cmd到fdset*/
		fd_wait(&skfds, &maxfd, g_sockfd_cmd);
#endif

		/*添加运行数据的socket链表中的connectfd到fdset*/
		event_add_clients(g_server_clients_data, &skfds, &maxfd);
#ifdef SOCKET_CMD_PORT
		/*添加命令下发的socket链表中的connectfd到fdset*/
		event_add_clients(g_server_clients_cmd, &skfds, &maxfd);
#endif
		ret = select(maxfd + 1, &skfds, NULL, NULL, 0);
		if (ret > 0)
		{
			/*sockfd_data是否有连接请求*/
			if (FD_ISSET(g_sockfd_data, &skfds))
			{
//				pthread_mutex_lock(&mutex);
				lgpe_Socket_Handle_Server_In(g_sockfd_data,	g_server_clients_data);
//				pthread_mutex_unlock(&mutex);
			}
#ifdef SOCKET_CMD_PORT
			/*sockfd_cmd是否有连接请求*/
			if (FD_ISSET(g_sockfd_cmd, &skfds))
			{
				pthread_mutex_lock(&mutex);
				lgpe_Socket_Handle_Server_In(g_sockfd_cmd, g_server_clients_cmd);
				pthread_mutex_unlock(&mutex);
			}
#endif

			/*检查命令下发socket端口是否有消息*/
			pthread_mutex_lock(&mutex);
#ifdef SOCKET_CMD_PORT
//			lgdebug(DBG_1,"lgpe_Socket_Recv_Thread: client_length:%d;client_head:%x;\n",
//					g_server_clients_cmd->length,(unsigned int) g_server_clients_cmd->head);
			event_check_clients(g_server_clients_cmd, &skfds,SOKCET_CMD);
			lgdebug(DBG_1,"lgpe_Socket_Recv_Thread: end cmd event_check_clients.\n");
#endif
			event_check_clients(g_server_clients_data, &skfds,SOCKET_DATA);
			pthread_mutex_unlock(&mutex);
			lgdebug(DBG_1,"lgpe_Socket_Recv_Thread: end data event_check_clients.\n");
		}
	}
}

/** ============================================================================
 *  @func   FD_SET fd to fdset and calculator maxfd
 *
 *  @desc   This function allocates and initializes resources used by
 *          this application.
 *  ============================================================================
 */
void fd_wait(fd_set *fds, int *maxfd, int fd)
{
	if (fd > *maxfd)
	{
		*maxfd = fd;
	}
	FD_SET(fd, fds);
}
LGPEConnElem* get_elem_by_fd(LGPEConnList* clist, int sfd)
{
	LGPEConnElem* elem = clist->head;
	while (elem != NULL)
	{
		if (elem->conn_fd == sfd)
			return elem;
		else
			elem = elem->next;
	}
	return NULL;
}
#if 0
void static sock_free_elem(LGPEConnElem* elem)
{
	struct packet_list *pack = elem->recv.queue;
	struct packet_list *dead;
	while(pack != NULL)
	{
		free(pack->packet);
		dead = pack;
		pack = pack->next;
		free(dead);
	}
	return;
}
#endif
void lgpe_Socket_Remove_Client(LGPEConnList* clist, int fd)
{
	LGPEConnElem* dead;
	LGPEConnElem* temp;

	if (clist->length == 0)
	{
		return;
	}

	if (clist->head->conn_fd == fd)
	{
		dead = clist->head;
		lgdebug(DBG_2, "lgpe_Socket_Remove_Client: Removing client %i from head \n",
				dead->conn_fd);
		clist->head = clist->head->next;
		clist->length--;
		if (clist->tail == dead)
			clist->tail = NULL;
		free(dead);
//		sock_free_elem(dead);
		return;
	}

	temp = clist->head;
	dead = temp->next;

	while (dead != NULL)
	{
		if (dead->conn_fd == fd)
		{
			lgdebug(DBG_2, "lgpe_Socket_Remove_Client: Removing client %i \n", dead->conn_fd);
			temp->next = dead->next;
			clist->length--;
			if (clist->tail == dead)
				clist->tail = temp;
//    		sock_free_elem(dead);
			free(dead);
			return;
		}
		temp = dead;
		dead = dead->next;
	}

}
int sock_read_byte(LGPEConnElem* elem)
{
	int n;
	n = read(elem->conn_fd, elem->recv.buffer, sizeof elem->recv.buffer);

	return n;
}
int sock_read_and_process(LGPEConnElem* elem)
{
//	int n;
//	n= read(elem->conn_fd, elem->buffer, sizeof(elem->buffer));
//	return n;

	int byte = sock_read_byte(elem);
	//CHNLGPE_1Print ("byte :%d \n", byte) ;
	if (byte <= 0)
	{
		perror("read_and_process: read error!");
		return -2;
	}
	return byte;
}
void lgpe_Socket_Handle_Client_In(LGPEConnList* clist, int connfd, int porttype)
{
	//gettimeofday(&g_pkt_start,NULL);
	int nRead;
	// socket
	LGPEConnElem *conn = get_elem_by_fd(clist, connfd);
	if ((nRead = sock_read_and_process(conn)) == -2)
	{
		CHNLGPE_0Print("lgpe_Socket_Handle_Client_In: disconnected.\n");
		lgpe_Socket_Remove_Client(clist, connfd);
		CHNLGPE_0Print("lgpe_Socket_Handle_Client_In: client removed.\n");
//		chnlSendData((char *)conn->recv.buffer,nRead);
	}
	else/*处理收到的数据*/
	{
		if(!porttype)
			return;
		CHNLGPE_0Print("lgpe_Socket_Handle_Client_In: socket recv.\n");
		if (ARM_AnalysisSocketData(conn->recv.buffer) == 1)
		{
			pthread_mutex_lock(&sql_mutex);
			//加入sql操作队列
			pthread_mutex_unlock(&sql_mutex);
			sem_post(&sem);
		}

//		chnlSendData(conn->recv.buffer, nRead);
	}
//  g_client_in_count ++;
//  printf("socket_in called: %d times\n",g_client_in_count);
//  putchar('\n');
//   socket
//  while(sock_packet_available(conn))
//  {
//    struct packet_list *entry = sock_pop_protocol_packet(conn);
//

//    write_serial_packet(g_serial_src,entry->packet,entry->len);
//
//    free(entry->packet);
//    free(entry);
//  }
//
//  gettimeofday(&g_pkt_end,NULL);
//  g_pkt_cpu_time_used = (g_pkt_end.tv_sec * 1000000 + g_pkt_end.tv_usec) - (g_pkt_start.tv_sec * 1000000 + g_pkt_start.tv_usec);
//  xdebug(XDBG_3,"Downstream Packet Processing Time = %li (usec) \n", g_pkt_cpu_time_used);
	return;
}
void event_check_clients(LGPEConnList* clist, fd_set* fds,int porttype)
{
	LGPEConnElem* elem;
	LGPEConnElem* temp;
	int nRead;
	if (clist->head == NULL || clist->length == 0)
		return;

//	for (elem = clist->head; elem != NULL; elem = elem->next)
	elem = clist->head;

	while(elem!=NULL)
	{
		if (FD_ISSET(elem->conn_fd, fds))
		{
//			printf("event_check_clients:elem get success\n");
//			lgpe_Socket_Handle_Client_In(clist, elem->conn_fd, porttype);
			if ((nRead = sock_read_and_process(elem)) == -2)
			{
				CHNLGPE_0Print("lgpe_Socket_Handle_Client_In: disconnected.\n");
				temp = elem->next;
				lgpe_Socket_Remove_Client(clist, elem->conn_fd);
				elem = temp;
				CHNLGPE_0Print("lgpe_Socket_Handle_Client_In: client removed.\n");
//				CHNLGPE_1Print("lgpe_Socket_Handle_Client_In: elem->next%d.\n",(int)elem->next);
				continue;
		//		chnlSendData((char *)conn->recv.buffer,nRead);
			}
			else/*处理收到的数据*/
			{
				if(!porttype)
				{
//					CHNLGPE_0Print("event_check_clients: data端口收到数据.\n");
					return;
				}
				CHNLGPE_0Print("lgpe_Socket_Handle_Client_In: socket recv.\n");
				if (ARM_AnalysisSocketData(elem->recv.buffer) == 1)
				{
									//CHNLGPE_0Print ("new data\n") ;
					LGPEsqloperateElem *opelem = (LGPEsqloperateElem *)malloc(sizeof(LGPEsqloperateElem));
					opelem->str = (unsigned char*)malloc(BUFSIZE);
					memcpy(opelem->str,elem->recv.buffer,BUFSIZE);
					pthread_mutex_lock(&sql_mutex);
					LGPE_sql_operate_add_elem(g_sqlOperate_list,opelem);
					pthread_mutex_unlock(&sql_mutex);
					sem_post(&sem);
				}

		//		chnlSendData(conn->recv.buffer, nRead);
			}
		}
		elem = elem->next;
	}
	return;
}
void lgpe_Socket_Handle_Server_In(int fd, LGPEConnList *clist)
{
	int sin_size = sizeof(struct sockaddr_in);
	bzero(&client_addr, sin_size);
	int client_fd = accept(fd, (struct sockaddr *) (&client_addr),
			(socklen_t*) &sin_size);
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	LGPEConnElem* client_elem;
//	int result;
//	struct timeval ntimeout;

	if (client_fd >= 0)
	{
		fprintf(stderr,
				"lgpe_Socket_Handle_Server_In: Server get connection from %s\n",
				inet_ntoa(client_addr.sin_addr));
#if 0
		ntimeout.tv_sec = 1;
		result = setsockopt(client_fd,SOL_SOCKET,SO_SNDTIMEO,
				(const void*) &ntimeout, sizeof(ntimeout));
		printf("lgpe_Socket_Handle_Server_In: socket send timeout result %d \n", result);

		ntimeout.tv_sec = 1;
		result = setsockopt(client_fd,SOL_SOCKET,SO_RCVTIMEO,
				(const void*) &ntimeout,sizeof(ntimeout));
		printf("lgpe_Socket_Handle_Server_In: socket recv timeout result %d \n", result);
#endif
		client_elem = lgpe_Socket_Create_Elem(client_fd);
		//add the client to the client list
		lgpe_Socket_Add_Client(clist, client_elem);
	}
}
LGPEConnElem* lgpe_Socket_Create_Elem(int cfd)
{
	LGPEConnElem* elem = (LGPEConnElem*) malloc(sizeof(LGPEConnElem));
	memset(elem, 0, sizeof(LGPEConnElem));
	elem->conn_fd = cfd;
	elem->recv.count = 0;
	elem->recv.bufpos = 0;
	elem->recv.bufused = 0;
	elem->next = NULL;
	return elem;
}
void lgpe_Socket_Add_Client(LGPEConnList* clist, LGPEConnElem* elem)
{
	if (clist->length == 0)
	{
		//xdebug(XDBG_2,"xservsocket: Adding new client %i \n", elem->conn_fd);
		clist->length++;
		clist->head = elem;
		clist->tail = elem;
		elem->next = NULL;
	}
	else
	{
		//xdebug(XDBG_2,"xservsocket: Adding new client %i \n", elem->conn_fd);
		clist->length++;
		clist->tail->next = elem;
		clist->tail = elem;
		elem->next = NULL;
	}
}
void event_add_clients(LGPEConnList* clist, fd_set* fds, int* maxfd)
{
	LGPEConnElem* elem;
	if (clist->head == NULL || clist->length == 0)
		return;
	for (elem = clist->head; elem != NULL; elem = elem->next)
	{
		fd_wait(fds, maxfd, elem->conn_fd);
	}
	return;
}
int safewrite(int fd, const void *buffer, int count)
{
	int actual = 0;

	while (count > 0)
	{
		int n = write(fd, buffer, count);

		if (n == -1 && errno == EINTR)
		{
			continue;
		}
		if (n == -1)
		{
			return -1;
		}

		count -= n;
		actual += n;
		buffer += n;
	}

	return actual;
}
int xsocket_write_packet(int fd, const void *packet, int len)
{
	// unsigned char l = (char)len;

	if (safewrite(fd, packet, len) != len)
	{
		return -1;
	}
	return 0;
}
void lgpe_Socket_Dispatch_To_Clients(LGPEConnList* clist, unsigned char* packet,
		int len)
{
	LGPEConnElem* elem;
	LGPEConnElem* temp;
	int result;
	elem = clist->head;
	if (elem == NULL)
		return;
	while (elem != NULL)
	{
//		printf("lgpe_Socket_Dispatch_To_Clients: sending %d bytes data to client %d.\n", len, elem->conn_fd);
		result = xsocket_write_packet(elem->conn_fd, packet, len);
//		printf("lgpe_Socket_Dispatch_To_Clients: sending data to client return %d.\n", result);
		if (result < 0)
		{
			temp = elem->next;
			lgpe_Socket_Remove_Client(clist, elem->conn_fd);
			elem = temp;
			// xdebug(XDBG_ERROR,"unable to write to client!\n");
			continue;
		}
		elem = elem->next;
	}
}

int lgpe_Socket_Open_Server_Port(int port)
{
	struct sockaddr_in me;
	int opt;
	int server_socket;

	server_socket = xassert_fatal("socket", socket(AF_INET, SOCK_STREAM, 0));
	//xassert_fatal("socket fcntl", fcntl(server_socket, F_SETFL, O_NONBLOCK));

	memset(&me, 0, sizeof me);
	me.sin_family = AF_INET;
	me.sin_port = htons(port);

	opt = 1;
	xassert_fatal("setsockopt",
			setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
					sizeof(opt)));

	xassert_fatal("bind",
			bind(server_socket, (struct sockaddr *) &me, sizeof me));
	xassert_fatal("listen", listen(server_socket, 5));
	return server_socket;
}

LGPEConnList* lgpe_Socket_Create_List(int sfd)
{
	LGPEConnList* list = (LGPEConnList*) malloc(sizeof(LGPEConnList));
	list->length = 0;
	list->server_fd = sfd;
	list->head = NULL;
	list->tail = NULL;
	return list;
}
void lgpe_Socket_Remove_List(LGPEConnList* clist)
{
	LGPEConnElem* elem;
	LGPEConnElem* temp;
	elem = clist->head;
	while (elem != NULL)
	{
		temp = elem->next;
		lgpe_Socket_Remove_Client(clist, elem->conn_fd);
		elem = temp;
	}
	free(clist);
	return;
}

