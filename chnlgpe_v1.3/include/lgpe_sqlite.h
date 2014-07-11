/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_sqlite.h
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  xhh  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_sqlite.h
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  xhh  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _LGPE_SQLITE_H_
#define _LGPE_SQLITE_H_

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*----------------------------------------------------------*
 * 调试开关             Debug Switch Section                		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件             Include File Section               	 	*
 *----------------------------------------------------------*/

#include"sqlite3.h"
#include"cJSON.h"

/*----------------------------------------------------------*
 * 全局宏定义           Global Macro Define Section         		*
 *----------------------------------------------------------*/
#define  PAGE_RECORD   				9
#define	 SELECTALLEVENT				0
#define	 SELECTOPERATEEVENT			1
#define	 SELECTFAULTEVENT			2
#define	 SELECTALARMEVENT			3
#define	 SELECTOPERATEFAULTEVENT	4
#define	 SELECTOPERATEALARMEVENT	5
#define	 SELECTFAULTALARMEVENT		6
/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     		*
 *----------------------------------------------------------*/
/* 数据库操作元素 */
typedef struct _sqliteoperateElem {
	unsigned char * str;
	void* next;
} LGPEsqloperateElem;

/* 数据库操作链表 */
typedef struct _sqliteoperateList {
	int length;
	LGPEsqloperateElem* head;
	LGPEsqloperateElem* tail;
} LGPEsqloperateList;
/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     		*
 *----------------------------------------------------------*/
//char *out;		//为编译通过 临时设里的变量
sqlite3 *db;
cJSON *json;
LGPEsqloperateList* g_sqlOperate_list;
/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    	*
 *----------------------------------------------------------*/
LGPEsqloperateList* LGPE_sql_operate_Create_List();

void LGPE_sql_operate_remove_list(LGPEsqloperateList* clist);

void LGPE_sql_operate_add_elem(LGPEsqloperateList* clist, LGPEsqloperateElem* elem);

void LGPE_sql_operate_Remove_elem(LGPEsqloperateList* clist,unsigned char *str);

void *thrd_Database();

void thrd_Sql_Handle();

void JsonAnalysis(cJSON *json);

int DbConnect(char *db_name);

void DbReturn(char *Cmd,char DeviceModel,char DeviceID,char Result);

int SelectEvent(cJSON *json,int selectMode);

//int SelectAllEvent(cJSON *json);
//
//int SelectOperate_Event(cJSON *json);
//
//int SelectFaultEvent(cJSON *json);

int PwdValidate(cJSON *json);

int SetUsrPwd(cJSON *json);

int SaveEvent(cJSON *json,int type);

int SetProjectParam(cJSON *json);

int SetGuardParam(cJSON *json);

int SetControlParam(cJSON *json);

int GetProjectParam(cJSON *json);

int GetGuardParam(cJSON *json);

int GetControlParam(cJSON *json);

void CreatePasswd_JSONStr(char *cmd,char *username,char *passwd,char **JSON_string);
/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   	*
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _LGPE_SQLITE_H_ */
