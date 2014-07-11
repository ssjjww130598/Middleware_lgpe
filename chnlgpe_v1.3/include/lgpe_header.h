/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_header.h
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_header.h
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  Chase  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _LGPE_HEADER_H_
#define _LGPE_HEADER_H_

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*----------------------------------------------------------*
 * 调试开关             Debug Switch Section                		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件             Include File Section                		*
 *----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <malloc.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include <mcheck.h>

/*dsplink header*/
#include <dsplink.h>
#include <proc.h>
#include <chnl.h>
#include <pool.h>
#if defined (DA8XXGEM)
#include <loaderdefs.h>
#endif

#include "cJSON.h"
#include "sqlite3.h"
//#include "sqlite3ext.h"
#include "lgpe_defines.h"
#include "lgpe_dsplink.h"
#include "lgpe_socket.h"
#include "lgpe_protocol.h"
#include "lgpe_utility.h"
#include "lgpe_sqlite.h"
/*----------------------------------------------------------*
 * 全局宏定义           Global Macro Define Section         		*
 *----------------------------------------------------------*/


/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    	*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   	*
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _LGPE_HEADER_H_ */
