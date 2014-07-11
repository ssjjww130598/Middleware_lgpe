/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_sqlite.c
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_sqlite.c
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  Chase  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _LGPE_SQLITE_C_
#define _LGPE_SQLITE_C_

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*----------------------------------------------------------*
 * 调试开关             Debug Switch Section                		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件             Include File Section                		*
 *----------------------------------------------------------*/
//#include <malloc.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include "sqlite3.h"
//#include "lgpe_sqlite.h"
#include "lgpe_header.h"
/*----------------------------------------------------------*
 * 全局宏定义           Global Macro Define Section         		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     		*
 *----------------------------------------------------------*/
char *EventReturnCmd = "Event_Return";
char *requstProjectParamCmd = "Request_Project_Param";
char *requstGUardParamCmd = "Request_Guard_Param";
char *requstControlParamCmd = "Request_Control_Param";
char *eventSelectReturnCmd = "Event_Select";
extern pthread_mutex_t sql_mutex,mutex;/*线程锁，控制sql处理队列*/
extern sem_t sem;				/*数据库队列操作信号量*/
/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    	*
 *----------------------------------------------------------*/
/*数据库操作队列创建*/
LGPEsqloperateList* LGPE_sql_operate_Create_List()
{
	LGPEsqloperateList* list = (LGPEsqloperateList*)malloc(sizeof(LGPEsqloperateList));
	list->length=0;
	list->head=NULL;
	list->tail=NULL;
	return list;
}
/*数据库操作队列移除*/
void LGPE_sql_operate_remove_list(LGPEsqloperateList* clist)
{
	LGPEsqloperateElem* elem;
	LGPEsqloperateElem* temp;
	elem = clist->head;
	while(elem != NULL)
	{
		temp = elem->next;
		free(elem->str);
		free(elem);
		elem = temp;
	}
	free(clist);
}
/*数据库操作队列添加新元素*/
void LGPE_sql_operate_add_elem(LGPEsqloperateList* clist, LGPEsqloperateElem* elem)
{
	if(clist->length == 0){
		clist->length++;
		clist->head = elem;
		clist->tail = elem;
		elem->next = NULL;
	}
	else{
		clist->length++;
		clist->tail->next = elem;
		clist->tail = elem;
		elem->next=NULL;
	}
}
/*数据库操作队列删除元素*/
void LGPE_sql_operate_Remove_elem(LGPEsqloperateList* clist,unsigned char *str)
{
	LGPEsqloperateElem* temp;
	LGPEsqloperateElem* dead;
	if (clist->length == 0)
	{
		return;
	}

	if (!strcmp(clist->head->str,str))
	{
		dead = clist->head;
		// xdebug(XDBG_2,"xservsocket: Removing client %i \n", dead->conn_fd);
		clist->head = clist->head->next;
		clist->length--;
		if (clist->tail == dead)
			clist->tail = NULL;

		free(dead->str);
		free(dead);
		//sock_free_elem(dead);
		return;
	}

	temp = clist->head;
	dead = temp->next;

	while (dead != NULL)
	{
		if (!strcmp(dead->str,str))
		{
			// xdebug(XDBG_2,"xservsocket: Removing client %i \n", dead->conn_fd);
			temp->next = dead->next;
			clist->length--;
			if (clist->tail == dead)
				clist->tail = temp;
			free(dead->str);
			free(dead);
			//    sock_free_elem(dead);
			return;
		}
		temp = dead;
		dead = dead->next;
	}
}
void *thrd_Database()
{

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);//���Ա��ر�
		for(;;)
		{

			sem_wait(&sem);
			CHNLGPE_0Print ("thrd_Database:new data\n") ;
			pthread_mutex_lock(&sql_mutex);
			thrd_Sql_Handle(g_sqlOperate_list->head->str);		//数据库操作函数
			CHNLGPE_0Print ("thrd_Database:handle over\n") ;
			LGPE_sql_operate_Remove_elem(g_sqlOperate_list,g_sqlOperate_list->head->str);
			pthread_mutex_unlock(&sql_mutex);

		}
}
/*****数据库线程处理函数 *****/
void thrd_Sql_Handle(char *out)
{

	json = cJSON_Parse(out);
//	free(out);
	if(!json)
	{
		printf("Error before:[%s]\n",cJSON_GetErrorPtr());
	}
	else
	{
		JsonAnalysis(json);
	}
	cJSON_Delete(json);

}

void JsonAnalysis(cJSON *json)
{
	char *Cmd;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DbConnect("chnlgpe_test.db");
	if(0 == strcmp(Cmd,"Select_All_Event"))
	{
		SelectEvent(json,SELECTALLEVENT);
	}
	else if(0 == strcmp(Cmd,"Select_Operate_Event"))
	{
		SelectEvent(json,SELECTOPERATEEVENT);
	}
	else if(0 == strcmp(Cmd,"Select_Fault_Event"))
	{
		SelectEvent(json,SELECTFAULTEVENT);
	}
	else if(0 == strcmp(Cmd,"Select_Alarm_Event"))
	{
		SelectEvent(json,SELECTALARMEVENT);
	}
	else if(0 == strcmp(Cmd,"Select_ Operate _ Fault _Event"))
	{
		SelectEvent(json,SELECTOPERATEFAULTEVENT);
	}
	else if(0 == strcmp(Cmd,"Select_ Operate _ Alarm _Event"))
	{
		SelectEvent(json,SELECTOPERATEALARMEVENT);
	}
	else if(0 == strcmp(Cmd,"Select_ Fault _ Alarm _Event"))
	{
		SelectEvent(json,SELECTFAULTALARMEVENT);
	}
	else if(0 == strcmp(Cmd,"Pwd_Validate"))
	{
		PwdValidate(json);
	}
	else if(0 == strcmp(Cmd,"Set_Usr_Pwd"))
	{
		SetUsrPwd(json);
	}
	else if(0 == strcmp(Cmd,"Save_Operate_Event"))
	{
		SaveEvent(json,0);
	}
	else if(0 == strcmp(Cmd,"Save_Fault_Event"))
	{
		SaveEvent(json,1);
	}
	else if(0 == strcmp(Cmd,"Set_Project_Param"))
	{
		SetProjectParam(json);
	}
	else if(0 == strcmp(Cmd,"Set_Guard_Param"))
	{
		SetGuardParam(json);
	}
	else if(0 == strcmp(Cmd,"Set_Control_Param"))
	{
		SetControlParam(json);
	}
	else if(0 == strcmp(Cmd,"Get_Project_Param"))
	{
		GetProjectParam(json);
	}
	else if(0 == strcmp(Cmd,"Get_Guard_Param"))
	{
		GetGuardParam(json);
	}
	else if(0 == strcmp(Cmd,"Get_Control_Param"))
	{
		GetControlParam(json);
	}
	sqlite3_close(db);
}

int DbConnect(char *db_name)
{
	int rc;
	char *zErrMsg = 0;
	rc = sqlite3_open(db_name,&db);
	if(rc != SQLITE_OK)
	{
		printf("zErrMsg = %s\n",zErrMsg);
		return -1;
	}
	return 0;
}

int SelectEvent(cJSON *json,int selectMode)
{
	char *zErrMsg = 0;
	char sql[256];
	char sqlRecord[256];
	int result;
	char **dbResult;
	int nrow = 0;
	int ncolumn = 0;
	sqlite3_stmt *stmt;

	cJSON *root,*r,*fld;
	char *out;
//	int rownum = 0;
	char DeviceModel;
	char DeviceID;
//	char TypeStr[10];
//	char TypeSel[10];
	int Type;
	int Page;
	int PageRecord;
	char *EventKey;
	char *strlike = "%";
	char *StartDate;
	char *EndDate;
	char *Cmd;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	Type = cJSON_GetObjectItem(json,"Select_Type")->valueint;
//	sprintf(TypeStr,"%d",Type);
	Page = cJSON_GetObjectItem(json,"Page")->valueint;
	PageRecord = cJSON_GetObjectItem(json,"Page_Record")->valueint;


	if(0 == Type)
	{
		switch(selectMode)
		{
			case SELECTALLEVENT:
				sprintf(sqlRecord,"select count(*) from EventRecTable");
				sprintf(sql,"select * from EventRecTable order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
				break;
			case SELECTOPERATEEVENT:
				sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 order by Type");
				sprintf(sql,"select * from EventRecTable where Type=0 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
				break;
			case SELECTFAULTEVENT:
				sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 order by Type");
				sprintf(sql,"select * from EventRecTable where Type=1 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
				break;
			case SELECTALARMEVENT:
				sprintf(sqlRecord,"select count(*) from EventRecTable where Type=2 order by Type");
				sprintf(sql,"select * from EventRecTable where Type=2 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
				break;
			case SELECTOPERATEFAULTEVENT:
				sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 or Type=1 order by Type");
				sprintf(sql,"select * from EventRecTable where Type=2 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
				break;
			case SELECTOPERATEALARMEVENT:
				sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 or Type=2 order by Type");
				sprintf(sql,"select * from EventRecTable where Type=2 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
				break;
			case SELECTFAULTALARMEVENT:
				sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 or Type=2 order by Type");
				sprintf(sql,"select * from EventRecTable where Type=2 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
				break;
			default:	break;
		}
	}
	else
	{
		EventKey = cJSON_GetObjectItem(json,"Event_Key")->valuestring;
		StartDate = cJSON_GetObjectItem(json,"Start_Date")->valuestring;
		EndDate = cJSON_GetObjectItem(json,"End_Date")->valuestring;
		if(0 != strcmp(EventKey,"全部"))
		{
			switch(selectMode)
			{
				case SELECTALLEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTOPERATEEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where Type=0 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTFAULTEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where Type=1 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTALARMEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where Type=2 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where Type=2 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTOPERATEFAULTEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 or Type=1 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where Type=0 or Type=1 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTOPERATEALARMEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 or Type=2 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where Type=0 or Type=2 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTFAULTALARMEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 or Type=2 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where Type=1 or Type=2 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				default:	break;
			}
		}
		else
		{
			switch(selectMode)
			{
				case SELECTALLEVENT:
					sprintf(sqlRecord,"select count(*) from EventRecTable where OccurTime between '%s' and '%s' order by Id DESC",StartDate,EndDate);
					sprintf(sql,"select * from EventRecTable where OccurTime between '%s' and '%s' order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTOPERATEEVENT:
		            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
		            sprintf(sql,"select * from EventRecTable where Type=0 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTFAULTEVENT:
		            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
		            sprintf(sql,"select * from EventRecTable where Type=1 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTALARMEVENT:
		            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=2 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
		            sprintf(sql,"select * from EventRecTable where Type=2 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTOPERATEFAULTEVENT:
		            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 or Type=1 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
		            sprintf(sql,"select * from EventRecTable where Type=0 or Type=1 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTOPERATEALARMEVENT:
		            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 or Type=2 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
		            sprintf(sql,"select * from EventRecTable where Type=0 or Type=2 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				case SELECTFAULTALARMEVENT:
		            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 or Type=2 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
		            sprintf(sql,"select * from EventRecTable where Type=1 or Type=2 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
					break;
				default:	break;

			}
		}
	}
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Cmd",EventReturnCmd);
	cJSON_AddNumberToObject(root,"DeviceModel",DeviceModel);
	cJSON_AddNumberToObject(root,"DeviceID",DeviceID);
	cJSON_AddNumberToObject(root,"Select_Type",Type);

    result=sqlite3_get_table(db, sqlRecord, &dbResult, &nrow, &ncolumn, &zErrMsg);
	if(SQLITE_OK == result)
	{
		if(nrow<1)
		{
			DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
			sqlite3_free_table(dbResult);
			return -1;
		}
		else
		{
			 //JSON记录数据条数
			cJSON_AddNumberToObject(root,"Records",atoi(dbResult[ncolumn]));
		}
	}
	else
	{
		DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		sqlite3_free_table(dbResult);
		return -1;
	}
	sqlite3_free_table(dbResult);
    // result=sqlite3_get_table(db, sql, &dbResult, &nrow, &ncolumn, &zErrMsg);
     result = sqlite3_prepare(db,sql,strlen(sql),&stmt,NULL);
	 if(SQLITE_OK==result)
     {
		 cJSON_AddItemToObject(root,"DbResult",r = cJSON_CreateArray());
    	 //结果加入到JSON 发送
//        if(0==nrow)
//           QMessageBox::information(this,tr("提示"),tr("目前没有相关事件记录！"),QMessageBox::Ok,QMessageBox::NoIcon);  //如果不正确,弹出警告对话框
         while(sqlite3_step(stmt) == SQLITE_ROW)
         {
        	 fprintf(stdout,"%d  ,%d  ,%s  ,%s\n",
        			 sqlite3_column_int(stmt,0),
        			 sqlite3_column_int(stmt,1),
        			 sqlite3_column_text(stmt,2),
        			 sqlite3_column_text(stmt,3)
        	  );
  //     	 sprintf(TypeSel,"%d",sqlite3_column_int(stmt,1));
        	 cJSON_AddItemToArray(r,fld = cJSON_CreateObject());
  //     	 cJSON_AddStringToObject(fld,"Id",sqlite3_column_int(stmt,0));
        	 cJSON_AddNumberToObject(fld,"Type",sqlite3_column_int(stmt,1));
        	 cJSON_AddStringToObject(fld,"Date",sqlite3_column_text(stmt,2));
        	 cJSON_AddStringToObject(fld,"Message",sqlite3_column_text(stmt,3));
         }
     }
	 else
	 {
		 DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		 sqlite3_finalize(stmt);
		 return -1;
	 }
	 out = cJSON_Print(root);	//out中保存的是JSON格式的字符串
	 pthread_mutex_lock(&mutex);
	 lgpe_Socket_Dispatch_To_Clients(g_server_clients_cmd,out,strlen(out));
	 pthread_mutex_unlock(&mutex);
	 cJSON_Delete(root);
	 free(out);
//	 printf("%s\n",out);
	 sqlite3_finalize(stmt);
	 return 1;
}


/*
int SelectAllEvent(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[256];
	char sqlRecord[256];
	int result;
	char **dbResult;
	int nrow = 0;
	int ncolumn = 0;
	sqlite3_stmt *stmt;

	cJSON *root,*r,*fld;
	char *out;
//	int rownum = 0;
	char DeviceModel;
	char DeviceID;
//	char TypeStr[10];
//	char TypeSel[10];
	int Type;
	int Page;
	int PageRecord;
	char *EventKey;
	char *strlike = "%";
	char *StartDate;
	char *EndDate;
	char *Cmd;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	Type = cJSON_GetObjectItem(json,"Select_Type")->valueint;
//	sprintf(TypeStr,"%d",Type);
	Page = cJSON_GetObjectItem(json,"Page")->valueint;
	PageRecord = cJSON_GetObjectItem(json,"Page_Record")->valueint;


	if(0 == Type)
	{
		sprintf(sqlRecord,"select count(*) from EventRecTable");
		sprintf(sql,"select * from EventRecTable order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
	}
	else
	{
		EventKey = cJSON_GetObjectItem(json,"Event_Key")->valuestring;
		StartDate = cJSON_GetObjectItem(json,"Start_Date")->valuestring;
		EndDate = cJSON_GetObjectItem(json,"End_Date")->valuestring;
		if(0 != strcmp(EventKey,"全部"))
		{
			sprintf(sqlRecord,"select count(*) from EventRecTable where Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
			sprintf(sql,"select * from EventRecTable where Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
		}
		else
		{
            sprintf(sqlRecord,"select count(*) from EventRecTable where OccurTime between '%s' and '%s' order by Id DESC",StartDate,EndDate);
            sprintf(sql,"select * from EventRecTable where OccurTime between '%s' and '%s' order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
		}
	}
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Cmd",allEventReturnCmd);
	cJSON_AddNumberToObject(root,"DeviceModel",DeviceModel);
	cJSON_AddNumberToObject(root,"DeviceID",DeviceID);
	cJSON_AddNumberToObject(root,"Select_Type",Type);

    result=sqlite3_get_table(db, sqlRecord, &dbResult, &nrow, &ncolumn, &zErrMsg);
	if(SQLITE_OK == result)
	{
		if(nrow<1)
		{
			DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
			sqlite3_free_table(dbResult);
			return -1;
		}
		else
		{
			 //JSON记录数据条数
			cJSON_AddNumberToObject(root,"Records",atoi(dbResult[ncolumn]));
		}
	}
	else
	{
		DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		sqlite3_free_table(dbResult);
		return -1;
	}
	sqlite3_free_table(dbResult);
    // result=sqlite3_get_table(db, sql, &dbResult, &nrow, &ncolumn, &zErrMsg);
     result = sqlite3_prepare(db,sql,strlen(sql),&stmt,NULL);
	 if(SQLITE_OK==result)
     {
		 cJSON_AddItemToObject(root,"DbResult",r = cJSON_CreateArray());
    	 //结果加入到JSON 发送
//        if(0==nrow)
//           QMessageBox::information(this,tr("提示"),tr("目前没有相关事件记录！"),QMessageBox::Ok,QMessageBox::NoIcon);  //如果不正确,弹出警告对话框
         while(sqlite3_step(stmt) == SQLITE_ROW)
         {
        	 fprintf(stdout,"%d  ,%d  ,%s  ,%s\n",
        			 sqlite3_column_int(stmt,0),
        			 sqlite3_column_int(stmt,1),
        			 sqlite3_column_text(stmt,2),
        			 sqlite3_column_text(stmt,3)
        	  );
  //     	 sprintf(TypeSel,"%d",sqlite3_column_int(stmt,1));
        	 cJSON_AddItemToArray(r,fld = cJSON_CreateObject());
  //     	 cJSON_AddStringToObject(fld,"Id",sqlite3_column_int(stmt,0));
        	 cJSON_AddNumberToObject(fld,"Type",sqlite3_column_int(stmt,1));
        	 cJSON_AddStringToObject(fld,"Date",sqlite3_column_text(stmt,2));
        	 cJSON_AddStringToObject(fld,"Message",sqlite3_column_text(stmt,3));
         }
     }
	 else
	 {
		 DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		 sqlite3_finalize(stmt);
		 return -1;
	 }
	 out = cJSON_Print(root);	//out中保存的是JSON格式的字符串
	 //		dispatch_packet_to_clients()
//	 dispatch_packet_to_clients(g_server_client,out,sizeof(out));
	 cJSON_Delete(root);
	 free(out);
//	 printf("%s\n",out);
	 sqlite3_finalize(stmt);
	 return 1;
}

int SelectOperate_Event(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[256];
	char sqlRecord[256];
	int result;
	char **dbResult;
	int nrow = 0;
	int ncolumn = 0;
	sqlite3_stmt *stmt;

	cJSON *root,*r,*fld;
	char *out;
	char DeviceModel;
	char DeviceID;
//	char TypeStr[10];
//	char TypeSel[10];
	int Type;
	int Page;
	int PageRecord;
	char *EventKey;
	char *strlike = "%";
	char *StartDate;
	char *EndDate;
	char *Cmd;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	Type = cJSON_GetObjectItem(json,"Select_Type")->valueint;
	Page = cJSON_GetObjectItem(json,"Page")->valueint;
	PageRecord = cJSON_GetObjectItem(json,"Page_Record")->valueint;
//	sprintf(TypeStr,"%d",Type);

	if(0 == Type)
	{
		sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 order by Type");
		sprintf(sql,"select * from EventRecTable where Type=0 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
	}
	else
	{
		EventKey = cJSON_GetObjectItem(json,"Event_Key")->valuestring;
		StartDate = cJSON_GetObjectItem(json,"Start_Date")->valuestring;
		EndDate = cJSON_GetObjectItem(json,"End_Date")->valuestring;
		if(0 != strcmp(EventKey,"全部"))
		{
			sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
			sprintf(sql,"select * from EventRecTable where Type=0 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
		}
		else
		{
            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=0 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
            sprintf(sql,"select * from EventRecTable where Type=0 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
		}
	}
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Cmd",operateEventReturn);
	cJSON_AddNumberToObject(root,"DeviceModel",DeviceModel);
	cJSON_AddNumberToObject(root,"DeviceID",DeviceID);
	cJSON_AddNumberToObject(root,"Select_Type",Type);
    result=sqlite3_get_table(db, sqlRecord, &dbResult, &nrow, &ncolumn, &zErrMsg);
	if(SQLITE_OK == result)
	{
		if(nrow<1)
		{
			DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
			sqlite3_free_table(dbResult);
			return -1;
		}
		else
		{
			 //JSON记录数据条数
			cJSON_AddNumberToObject(root,"Records",atoi(dbResult[ncolumn]));
		}
	}
	else
	{
		DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		sqlite3_free_table(dbResult);
		return -1;
	}
	sqlite3_free_table(dbResult);
    // result=sqlite3_get_table(db, sql, &dbResult, &nrow, &ncolumn, &zErrMsg);
     result = sqlite3_prepare(db,sql,strlen(sql),&stmt,NULL);
	 if(SQLITE_OK==result)
     {
		 cJSON_AddItemToObject(root,"DbResult",r = cJSON_CreateArray());
    	 //结果加入到JSON 发送
//        if(0==nrow)
//           QMessageBox::information(this,tr("提示"),tr("目前没有相关事件记录！"),QMessageBox::Ok,QMessageBox::NoIcon);  //如果不正确,弹出警告对话框
         while(sqlite3_step(stmt) == SQLITE_ROW)
         {
        	 fprintf(stdout,"%d  ,%d  ,%s  ,%s\n",
        			 sqlite3_column_int(stmt,0),
        			 sqlite3_column_int(stmt,1),
        			 sqlite3_column_text(stmt,2),
        			 sqlite3_column_text(stmt,3)
        	  );
   //     	 sprintf(TypeSel,"%d",sqlite3_column_int(stmt,1));
        	 cJSON_AddItemToArray(r,fld = cJSON_CreateObject());
  //     	 cJSON_AddStringToObject(fld,"Id",sqlite3_column_int(stmt,0));
        	 cJSON_AddNumberToObject(fld,"Type",sqlite3_column_int(stmt,1));
        	 cJSON_AddStringToObject(fld,"Date",sqlite3_column_text(stmt,2));
        	 cJSON_AddStringToObject(fld,"Message",sqlite3_column_text(stmt,3));
         }
     }
	 else
	 {
		 DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		 sqlite3_finalize(stmt);
		 return -1;
	 }
	 out = cJSON_Print(root);	//out中保存的是JSON格式的字符串
	 //		dispatch_packet_to_clients()
//	 dispatch_packet_to_clients(g_server_client,out,sizeof(out));
	 cJSON_Delete(root);
	 free(out);
//	 printf("%s\n",out);
	 sqlite3_finalize(stmt);
	 return 1;
}

int SelectFaultEvent(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[256];
	char sqlRecord[256];
	int result;
	char **dbResult;
	int nrow = 0;
	int ncolumn = 0;
	sqlite3_stmt *stmt;

	cJSON *root,*r,*fld;
	char *out;
	char DeviceModel;
	char DeviceID;
//	char TypeStr[10];
//	char TypeSel[10];
	int Type;
	int Page;
	int PageRecord;
	char *EventKey;
	char *strlike = "%";
	char *StartDate;
	char *EndDate;
	char *Cmd;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	Type = cJSON_GetObjectItem(json,"Select_Type")->valueint;
	Page = cJSON_GetObjectItem(json,"Page")->valueint;
	PageRecord = cJSON_GetObjectItem(json,"Page_Record")->valueint;
//	sprintf(TypeStr,"%d",Type);

	if(0 == Type)
	{
		sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 order by Type");
		sprintf(sql,"select * from EventRecTable where Type=1 order by Id DESC limit %d,%d",PageRecord*Page,PageRecord);
	}
	else
	{
		EventKey = cJSON_GetObjectItem(json,"Event_Key")->valuestring;
		StartDate = cJSON_GetObjectItem(json,"Start_Date")->valuestring;
		EndDate = cJSON_GetObjectItem(json,"End_Date")->valuestring;
		if(0 != strcmp(EventKey,"全部"))
		{
			sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC",strlike,EventKey,strlike,StartDate,EndDate);
			sprintf(sql,"select * from EventRecTable where Type=1 and Message like '%s%s%s' and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",strlike,EventKey,strlike,StartDate,EndDate,PageRecord*Page,PageRecord);
		}
		else
		{
            sprintf(sqlRecord,"select count(*) from EventRecTable where Type=1 and (OccurTime between '%s' and '%s') order by Id DESC",StartDate,EndDate);
            sprintf(sql,"select * from EventRecTable where Type=1 and (OccurTime between '%s' and '%s') order by Id DESC limit %d,%d",StartDate,EndDate,PageRecord*Page,PageRecord);
		}
	}
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Cmd",faultEventReturn);
	cJSON_AddNumberToObject(root,"DeviceModel",DeviceModel);
	cJSON_AddNumberToObject(root,"DeviceID",DeviceID);
	cJSON_AddNumberToObject(root,"Select_Type",Type);
    result=sqlite3_get_table(db, sqlRecord, &dbResult, &nrow, &ncolumn, &zErrMsg);
	if(SQLITE_OK == result)
	{
		if(nrow<1)
		{
			DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
			sqlite3_free_table(dbResult);
			return -1;
		}
		else
		{
			 //JSON记录数据条数
		  cJSON_AddNumberToObject(root,"Records",atoi(dbResult[ncolumn]));
		}
	}
	else
	{
		DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		sqlite3_free_table(dbResult);
		return -1;
	}
	sqlite3_free_table(dbResult);
    // result=sqlite3_get_table(db, sql, &dbResult, &nrow, &ncolumn, &zErrMsg);
     result = sqlite3_prepare(db,sql,strlen(sql),&stmt,NULL);
	 if(SQLITE_OK==result)
     {
		 cJSON_AddItemToObject(root,"DbResult",r = cJSON_CreateArray());
    	 //结果加入到JSON 发送
//        if(0==nrow)
//           QMessageBox::information(this,tr("提示"),tr("目前没有相关事件记录！"),QMessageBox::Ok,QMessageBox::NoIcon);  //如果不正确,弹出警告对话框
         while(sqlite3_step(stmt) == SQLITE_ROW)
         {
        	 fprintf(stdout,"%d  ,%d  ,%s  ,%s\n",
        			 sqlite3_column_int(stmt,0),
        			 sqlite3_column_int(stmt,1),
        			 sqlite3_column_text(stmt,2),
        			 sqlite3_column_text(stmt,3)
        	  );
  //      	 sprintf(TypeSel,"%d",sqlite3_column_int(stmt,1));
        	 cJSON_AddItemToArray(r,fld = cJSON_CreateObject());
  //     	 cJSON_AddStringToObject(fld,"Id",sqlite3_column_int(stmt,0));
        	 cJSON_AddNumberToObject(fld,"Type",sqlite3_column_int(stmt,1));
        	 cJSON_AddStringToObject(fld,"Date",sqlite3_column_text(stmt,2));
        	 cJSON_AddStringToObject(fld,"Message",sqlite3_column_text(stmt,3));
         }
     }
	 else
	 {
		 DbReturn(eventSelectReturnCmd,DeviceModel,DeviceID,0x00);
		 sqlite3_finalize(stmt);
		 return -1;
	 }
	 out = cJSON_Print(root);	//out中保存的是JSON格式的字符串
	 //		dispatch_packet_to_clients()
//	 dispatch_packet_to_clients(g_server_client,out,sizeof(out));
	 cJSON_Delete(root);
	 free(out);
//	 printf("%s\n",out);
	 sqlite3_finalize(stmt);
	 return 1;
}
*/
int PwdValidate(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[128];
	int result;
	char **dbResult;
	int nrow = 0;
	int ncolumn = 0;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	char *UsrName;
	char *Passwd;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	UsrName = cJSON_GetObjectItem(json,"UsrName")->valuestring;
    Passwd = cJSON_GetObjectItem(json,"PassWord")->valuestring;
    printf("%d\n%d\n%s\n",DeviceModel,DeviceID,Cmd);
    if(DeviceModel == 0x11)
    {
    	if(DeviceID == 0x00)
    	{
			sprintf(sql,"select count(*) from UsrTable where UsrName='%s' and Passwd='%s'",UsrName,Passwd);
			result=sqlite3_get_table(db, sql, &dbResult, &nrow, &ncolumn, &zErrMsg);

			if(SQLITE_OK==result)
			{
				if(nrow>0)
				{
					printf("dbResult[%d]:%s\n",ncolumn,dbResult[ncolumn]);

					if(atoi(dbResult[ncolumn])==1)
					{
					  // 找到匹配的用户
						printf("success\n");
						DbReturn(Cmd,DeviceModel,DeviceID,0x01);
						sqlite3_free_table(dbResult);
						return 1;
					}
					else
					{
						printf("fail\n");
						DbReturn(Cmd,DeviceModel,DeviceID,0x00);
						sqlite3_free_table(dbResult);
						return 1;
					}
				}
				sqlite3_free_table(dbResult);
				return -1;
			}
			else
			{
				DbReturn(Cmd,DeviceModel,DeviceID,0x00);
				sqlite3_free_table(dbResult);
				return -1;
			}
			printf("3\n");
    	}
    }
    return 1;
}

int SetUsrPwd(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[128];
	int result;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	char *UsrName;
	char *Passwd;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	UsrName = cJSON_GetObjectItem(json,"UsrName")->valuestring;
    Passwd = cJSON_GetObjectItem(json,"PassWord")->valuestring;
	sprintf(sql,"update UsrTable set Passwd='%s' where UsrName='%s'",Passwd,UsrName);
	result = sqlite3_exec(db,sql, NULL, NULL, &zErrMsg);
	if(SQLITE_OK==result)
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x01);
		return 1;
	}
	else
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x00);
	  	return -1;
	}
}

int SaveEvent(cJSON *json,int type)
{
	char *zErrMsg = 0;
	char sql[128];
	int result;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	char *Message;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
    Message = cJSON_GetObjectItem(json,"Message")->valuestring;

	sprintf(sql,"insert into EventRecTable(Type,Message) values(%d,'%s');",type,Message);
  	result = sqlite3_exec(db,sql, NULL, NULL, &zErrMsg);

	if(SQLITE_OK==result)
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x01);
		return 1;
	}
	else
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x00);
		return -1;
	}

}

int SetProjectParam(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[256];
	int result;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	float Frequency;
	float UnetRated;
	float IsvgRated;
	float PTnet;
	float PTnetH;
	float CTnet;
	float CTsvg;
	float Lsvg;
	float ChgStopVoltage;
	float DischgStopVoltage;
	int PowUnitNum;
	int PowUnitRedNum;

	Cmd         = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID    = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	Frequency   = cJSON_GetObjectItem(json,"Frequency")->valuedouble;
	UnetRated   = cJSON_GetObjectItem(json,"UnetRated")->valuedouble;
	IsvgRated   = cJSON_GetObjectItem(json,"IsvgRated")->valuedouble;
	PTnet       = cJSON_GetObjectItem(json,"PTnet")->valuedouble;
	PTnetH      = cJSON_GetObjectItem(json,"PTnetH")->valuedouble;
	CTnet       = cJSON_GetObjectItem(json,"CTnet")->valuedouble;
	CTsvg       = cJSON_GetObjectItem(json,"CTsvg")->valuedouble;
	Lsvg        = cJSON_GetObjectItem(json,"Lsvg")->valuedouble;
	ChgStopVoltage   = cJSON_GetObjectItem(json,"ChgStopVoltage")->valuedouble;
	DischgStopVoltage = cJSON_GetObjectItem(json,"DischgStopVoltage")->valuedouble;
	PowUnitNum    = cJSON_GetObjectItem(json,"PowUnitNum")->valueint;
	PowUnitRedNum = cJSON_GetObjectItem(json,"PowUnitRedNum")->valueint;
	sprintf(sql,"insert into EngParamTable(Frequency,UnetRated,IsvgRated,PTnet,PTnetH,CTnet,CTsvg,Lsvg,ChargeStopVoltage,DischargeStopVoltage,PowerUnitNum,PowerUnitRedundantNum) values('%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%d','%d');",
			Frequency,UnetRated,IsvgRated,PTnet,PTnetH,CTnet,CTsvg,Lsvg,ChgStopVoltage,DischgStopVoltage,PowUnitNum,PowUnitRedNum);
	result=sqlite3_exec(db,sql, NULL, NULL, &zErrMsg);
	if(SQLITE_OK==result)
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x01);
		return 1;
	}
	else
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x00);
		return -1;
	}
}

int SetGuardParam(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[512];
	int result;
	cJSON *format;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	char *VoltageGuardlevel;
	char *CurrentGuardlevel;
	char *FreqGuardlevel;
	char *DcVoltageGuardlevel;
	char *DcCurrentGuardleve;
	char *PowerunitGuardlevel;
	float OverVoltageK;
	float UnderVoltageK;
	float UnbalanceK;
	float OverCurrentK0;
	float OverCurrentK1;
	float OverCurrentK2;
	float OverCurrentDelay1;
	float OverCurrentDelay2;
	float UpperFreqLimit;
	float LowerFreqLimit;
	float PhaseShiftLimit;
	float DCOverVoltageK0;
	float DCOverVoltageK1;
	float DCUnderVoltageK;
	float DCUnbalanceK;
	float DCOverCurrentK0;
	float DCOverCurrentK1;
	float DCOverCurrentK2;
	float DCOverCurrentDelay1;
	float DCOverCurrentDelay2;
	float TemperatureLimit0;
	float TemperatureLimit1;

	Cmd = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	VoltageGuardlevel = cJSON_GetObjectItem(json,"VolGrdlevel")->valuestring;
	CurrentGuardlevel = cJSON_GetObjectItem(json,"CurtGrdlevel")->valuestring;
	FreqGuardlevel = cJSON_GetObjectItem(json,"FreqGrdlevel")->valuestring;
	DcVoltageGuardlevel = cJSON_GetObjectItem(json,"DcVolGrdlevel")->valuestring;
	DcCurrentGuardleve = cJSON_GetObjectItem(json,"DcCurtGrdleve")->valuestring;
	PowerunitGuardlevel = cJSON_GetObjectItem(json,"PowUnitGrdlevel")->valuestring;
	format = cJSON_GetObjectItem(json,"VoltageParam");
	OverVoltageK = cJSON_GetObjectItem(format,"OverVoltagek")->valuedouble;
	UnderVoltageK = cJSON_GetObjectItem(format,"UnderVoltagek")->valuedouble;
	UnbalanceK = cJSON_GetObjectItem(format,"Unbalancek")->valuedouble;
	format = cJSON_GetObjectItem(json,"CurrentParam");
	OverCurrentK0 = cJSON_GetObjectItem(format,"OverCurrentk0")->valuedouble;
	OverCurrentK1 = cJSON_GetObjectItem(format,"OverCurrentk1")->valuedouble;
	OverCurrentK2 = cJSON_GetObjectItem(format,"OverCurrentk2")->valuedouble;
	OverCurrentDelay1 = cJSON_GetObjectItem(format,"OverCurrent_Delay1")->valuedouble;
	OverCurrentDelay2 = cJSON_GetObjectItem(format,"OverCurrent_Delay2")->valuedouble;
	format = cJSON_GetObjectItem(json,"FreqParam");
	UpperFreqLimit = cJSON_GetObjectItem(format,"UpFreqLimit")->valuedouble;
	LowerFreqLimit = cJSON_GetObjectItem(format,"LowFreqLimit")->valuedouble;
	PhaseShiftLimit = cJSON_GetObjectItem(format,"PhaseShiftLimit")->valuedouble;
	format = cJSON_GetObjectItem(json,"DcVoltageParam");
	DCOverVoltageK0 = cJSON_GetObjectItem(format,"ServerOverVoltagek")->valuedouble;
	DCOverVoltageK1 = cJSON_GetObjectItem(format,"OverVoltagek")->valuedouble;
	DCUnderVoltageK = cJSON_GetObjectItem(format,"UnderVoltagek")->valuedouble;
	DCUnbalanceK = cJSON_GetObjectItem(format,"Unbalancek")->valuedouble;
	format = cJSON_GetObjectItem(json,"DcCurrentParam");
	DCOverCurrentK0 = cJSON_GetObjectItem(format,"OverCurrentk0")->valuedouble;
	DCOverCurrentK1 = cJSON_GetObjectItem(format,"OverCurrentk1")->valuedouble;
	DCOverCurrentK2 = cJSON_GetObjectItem(format,"OverCurrentk2")->valuedouble;
	DCOverCurrentDelay1 = cJSON_GetObjectItem(format,"OverCurrent_Delay1")->valuedouble;
	DCOverCurrentDelay2 = cJSON_GetObjectItem(format,"OverCurrent_Delay2")->valuedouble;
	format = cJSON_GetObjectItem(json,"PowUnitParam");
	TemperatureLimit0 = cJSON_GetObjectItem(format,"TemperLimit0")->valuedouble;
	TemperatureLimit1 = cJSON_GetObjectItem(format,"TemperLimit1")->valuedouble;


	sprintf(sql,"insert into ProParamTable(VoltageGuardlevel,CurrentGuardlevel,FreqGuardlevel,DcVoltageGuardlevel,DcCurrentGuardlevel,PowerunitGuardlevel,OverVoltageK,UnderVoltageK,UnbalanceK,OverCurrentK0,OverCurrentK1,OverCurrentK2,OverCurrentDelay1,OverCurrentDelay2,UpperFreqLimit,LowerFreqLimit,PhaseShiftLimit,DCOverVoltageK0,DCOverVoltageK1,DCUnderVoltageK,DCUnbalanceK,DCOverCurrentK0,DCOverCurrentK1,DCOverCurrentK2,DCOverCurrentDelay1,DCOverCurrentDelay2,TemperatureLimit0,TemperatureLimit1) values('%s','%s','%s','%s','%s','%s','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%f');",
			VoltageGuardlevel,CurrentGuardlevel,FreqGuardlevel,DcVoltageGuardlevel,DcCurrentGuardleve,PowerunitGuardlevel,OverVoltageK,UnderVoltageK,UnbalanceK,OverCurrentK0,OverCurrentK1,OverCurrentK2,OverCurrentDelay1,OverCurrentDelay2,UpperFreqLimit,LowerFreqLimit,PhaseShiftLimit,DCOverVoltageK0,DCOverVoltageK1,DCUnderVoltageK,DCUnbalanceK,DCOverCurrentK0,DCOverCurrentK1,DCOverCurrentK2,DCOverCurrentDelay1,DCOverCurrentDelay2,TemperatureLimit0,TemperatureLimit1);
	result=sqlite3_exec(db,sql, NULL, NULL, &zErrMsg);
	if(SQLITE_OK==result)
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x01);
		return 1;
	}
	else
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x00);
		return -1;
	}
}

int SetControlParam(cJSON *json)
{
	char *zErrMsg = 0;
	char sql[256];
	int result;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	float VbusRef;
	float VbusBase;
	float VcapRef;
	float VcapBase;
	float VbusKp;
	float VbusKi;
	float VcapKp;
	float VcapKi;
	float IdKp;
	float IdKi;
	int IqKp;
	int IqKi;

	Cmd         = cJSON_GetObjectItem(json,"Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json,"DeviceModel")->valueint;
	DeviceID    = cJSON_GetObjectItem(json,"DeviceID")->valueint;
	VbusRef     = cJSON_GetObjectItem(json,"VbusRef")->valuedouble;
	VbusBase    = cJSON_GetObjectItem(json,"VbusBase")->valuedouble;
	VcapRef     = cJSON_GetObjectItem(json,"VcapRef")->valuedouble;
	VcapBase    = cJSON_GetObjectItem(json,"VcapBase")->valuedouble;
	VbusKp      = cJSON_GetObjectItem(json,"VbusKp")->valuedouble;
	VbusKi      = cJSON_GetObjectItem(json,"VbusKi")->valuedouble;
	VcapKp      = cJSON_GetObjectItem(json,"VcapKp")->valuedouble;
	VcapKi      = cJSON_GetObjectItem(json,"VcapKi")->valuedouble;
	IdKp        = cJSON_GetObjectItem(json,"IdKp")->valuedouble;
	IdKi        = cJSON_GetObjectItem(json,"IdKi")->valuedouble;
	IqKp        = cJSON_GetObjectItem(json,"IqKp")->valueint;
	IqKi        = cJSON_GetObjectItem(json,"IqKi")->valueint;
	sprintf(sql,"insert into CtrParamTable(VbusRef,VbusBase,VcapRef,VcapBase,VbusKp,VbusKi,VcapKp,VcapKi,IdKp,IdKi,IqKp,IqKi) values('%f','%f','%f','%f','%f','%f','%f','%f','%f','%f','%d','%d');",
			VbusRef,VbusBase,VcapRef,VcapBase,VbusKp,VbusKi,VcapKp,VcapKi,IdKp,IdKi,IqKp,IqKi);
	result=sqlite3_exec(db,sql, NULL, NULL, &zErrMsg);
	if(SQLITE_OK==result)
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x01);
		return 1;
	}
	else
	{
		DbReturn(Cmd,DeviceModel,DeviceID,0x00);
		return -1;
	}
}

void DbReturn(char *Cmd,char DeviceModel,char DeviceID,char Result)
{
	cJSON *root;
	char *out;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Cmd",Cmd);
	cJSON_AddNumberToObject(root,"DeviceModel",DeviceModel);
	cJSON_AddNumberToObject(root,"DeviceID",DeviceID);
	cJSON_AddNumberToObject(root,"Result",Result);
	if(0x01 == Result)
	{
		out = cJSON_Print(root);
		pthread_mutex_lock(&mutex);
		lgpe_Socket_Dispatch_To_Clients(g_server_clients_cmd, out, strlen(out));
		pthread_mutex_unlock(&mutex);
		printf("\n%s\n",out);
		cJSON_Delete(root);
		free(out);
	}
	else
	{
		cJSON_AddNumberToObject(root,"ErrCode",0x01);
		out = cJSON_Print(root);
		pthread_mutex_lock(&mutex);
		lgpe_Socket_Dispatch_To_Clients(g_server_clients_cmd, out, strlen(out));
		pthread_mutex_unlock(&mutex);
		printf("\n%s\n",out);
		cJSON_Delete(root);
		free(out);
	}

}

int GetProjectParam(cJSON *json)
{
//	char *zErrMsg = 0;
	char sql[256];
	int result;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	sqlite3_stmt *stmt;

	cJSON *root, *r, *fld;
	char *out;

	Cmd = cJSON_GetObjectItem(json, "Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json, "DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json, "DeviceID")->valueint;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Cmd", requstProjectParamCmd);
	cJSON_AddNumberToObject(root, "DeviceModel", DeviceModel);
	cJSON_AddNumberToObject(root, "DeviceID", DeviceID);
	sprintf(sql, "select * from EngParamTable");

	result = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
	if (SQLITE_OK == result)
	{
		cJSON_AddItemToObject(root, "DbResult", r = cJSON_CreateArray());

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{

			/*      	 fprintf(stdout,"%d  ,%d  ,%s  ,%s\n",
			 sqlite3_column_int(stmt,0),
			 sqlite3_column_int(stmt,1),
			 sqlite3_column_text(stmt,2),
			 sqlite3_column_text(stmt,3)
			 );
			 */
			cJSON_AddItemToArray(r, fld = cJSON_CreateObject());
			//     	 cJSON_AddStringToObject(fld,"Id",sqlite3_column_int(stmt,0));
			cJSON_AddNumberToObject(fld, "Frequency ",
					sqlite3_column_double(stmt, 1));
			cJSON_AddNumberToObject(fld, "UnetRated",
					sqlite3_column_double(stmt, 2));
			cJSON_AddNumberToObject(fld, "IsvgRated",
					sqlite3_column_double(stmt, 3));
			cJSON_AddNumberToObject(fld, "PTnet",
					sqlite3_column_double(stmt, 4));
			cJSON_AddNumberToObject(fld, "PTnetH",
					sqlite3_column_double(stmt, 5));
			cJSON_AddNumberToObject(fld, "CTnet",
					sqlite3_column_double(stmt, 6));
			cJSON_AddNumberToObject(fld, "CTsvg",
					sqlite3_column_double(stmt, 7));
			cJSON_AddNumberToObject(fld, "Lsvg",
					sqlite3_column_double(stmt, 8));
			cJSON_AddNumberToObject(fld, "ChgStopVoltage",
					sqlite3_column_double(stmt, 9));
			cJSON_AddNumberToObject(fld, "DischgStopVoltage",
					sqlite3_column_double(stmt, 10));
			cJSON_AddNumberToObject(fld, "PowUnitNum",
					sqlite3_column_int(stmt, 11));
			cJSON_AddNumberToObject(fld, "PowUnitRedNum",
					sqlite3_column_int(stmt, 12));
		}
	}
	else
	{
		DbReturn(Cmd, DeviceModel, DeviceID, 0x00);
		sqlite3_finalize(stmt);
		return -1;
	}
	out = cJSON_Print(root);	//out中保存的是JSON格式的字符串
	//		dispatch_packet_to_clients()
	pthread_mutex_lock(&mutex);
	lgpe_Socket_Dispatch_To_Clients(g_server_clients_cmd, out, strlen(out));
	pthread_mutex_unlock(&mutex);
	cJSON_Delete(root);
	free(out);
//	 printf("%s\n",out);
	sqlite3_finalize(stmt);
	return 1;
}

int GetGuardParam(cJSON *json)
{
//	char *zErrMsg = 0;
	char sql[256];
	int result;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	sqlite3_stmt *stmt;

	cJSON *root, *r, *fld, *fmt;
	char *out;

	Cmd = cJSON_GetObjectItem(json, "Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json, "DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json, "DeviceID")->valueint;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Cmd", requstGUardParamCmd);
	cJSON_AddNumberToObject(root, "DeviceModel", DeviceModel);
	cJSON_AddNumberToObject(root, "DeviceID", DeviceID);
	sprintf(sql, "select * from ProParamTable");

	result = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
	if (SQLITE_OK == result)
	{
		cJSON_AddItemToObject(root, "DbResult", r = cJSON_CreateArray());

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{

			/*      	 fprintf(stdout,"%d  ,%d  ,%s  ,%s\n",
			 sqlite3_column_int(stmt,0),
			 sqlite3_column_int(stmt,1),
			 sqlite3_column_text(stmt,2),
			 sqlite3_column_text(stmt,3)
			 );
			 */
			cJSON_AddItemToArray(r, fld = cJSON_CreateObject());
			//     	 cJSON_AddStringToObject(fld,"Id",sqlite3_column_int(stmt,0));
			cJSON_AddNumberToObject(fld, "VolGrdlevel",
					sqlite3_column_int(stmt, 1));
			cJSON_AddNumberToObject(fld, "CurtGrdlevel",
					sqlite3_column_int(stmt, 2));
			cJSON_AddNumberToObject(fld, "FreqGrdlevel",
					sqlite3_column_int(stmt, 3));
			cJSON_AddNumberToObject(fld, "DcVolGrdlevel",
					sqlite3_column_int(stmt, 4));
			cJSON_AddNumberToObject(fld, "DcCurtGrdlevel",
					sqlite3_column_int(stmt, 5));
			cJSON_AddNumberToObject(fld, "PowUnitGrdlevel",
					sqlite3_column_int(stmt, 6));
			cJSON_AddItemToObject(fld, "VoltageParam", fmt =
					cJSON_CreateObject());
			cJSON_AddNumberToObject(fmt, "OverVoltagek",
					sqlite3_column_double(stmt, 7));
			cJSON_AddNumberToObject(fmt, "UnderVoltagek",
					sqlite3_column_double(stmt, 8));
			cJSON_AddNumberToObject(fmt, "Unbalancek",
					sqlite3_column_double(stmt, 9));
			cJSON_AddItemToObject(fld, "CurrentParam", fmt =
					cJSON_CreateObject());
			cJSON_AddNumberToObject(fmt, "OverCurrentk0",
					sqlite3_column_double(stmt, 10));
			cJSON_AddNumberToObject(fmt, "OverCurrentk1",
					sqlite3_column_double(stmt, 11));
			cJSON_AddNumberToObject(fmt, "OverCurrentk2",
					sqlite3_column_double(stmt, 12));
			cJSON_AddNumberToObject(fmt, "OverCurrent_Delay1",
					sqlite3_column_double(stmt, 13));
			cJSON_AddNumberToObject(fmt, "OverCurrent_Delay2",
					sqlite3_column_double(stmt, 14));
			cJSON_AddItemToObject(fld, "FreqParam", fmt = cJSON_CreateObject());
			cJSON_AddNumberToObject(fmt, "UpFreqLimit",
					sqlite3_column_double(stmt, 15));
			cJSON_AddNumberToObject(fmt, "LowFreqLimit",
					sqlite3_column_double(stmt, 16));
			cJSON_AddNumberToObject(fmt, "PhaseShiftLimit",
					sqlite3_column_double(stmt, 17));
			cJSON_AddItemToObject(fld, "DcVoltageParam", fmt =
					cJSON_CreateObject());
			cJSON_AddNumberToObject(fmt, "ServerOverVoltagek",
					sqlite3_column_double(stmt, 18));
			cJSON_AddNumberToObject(fmt, "OverVoltagek",
					sqlite3_column_double(stmt, 19));
			cJSON_AddNumberToObject(fmt, "UnderVoltagek",
					sqlite3_column_double(stmt, 20));
			cJSON_AddNumberToObject(fmt, "Unbalancek",
					sqlite3_column_double(stmt, 21));
			cJSON_AddItemToObject(fld, "DcCurrentParam", fmt =
					cJSON_CreateObject());
			cJSON_AddNumberToObject(fmt, "OverCurrentk0",
					sqlite3_column_double(stmt, 22));
			cJSON_AddNumberToObject(fmt, "OverCurrentk1",
					sqlite3_column_double(stmt, 23));
			cJSON_AddNumberToObject(fmt, "OverCurrentk2",
					sqlite3_column_double(stmt, 24));
			cJSON_AddNumberToObject(fmt, "OverCurrent_Delay1",
					sqlite3_column_double(stmt, 25));
			cJSON_AddNumberToObject(fmt, "OverCurrent_Delay2",
					sqlite3_column_double(stmt, 26));
			cJSON_AddItemToObject(fld, "PowUnitParam", fmt =
					cJSON_CreateObject());
			cJSON_AddNumberToObject(fmt, "TemperLimit0",
					sqlite3_column_double(stmt, 27));
			cJSON_AddNumberToObject(fmt, "TemperLimit1",
					sqlite3_column_double(stmt, 28));
		}
	}
	else
	{
		DbReturn(Cmd, DeviceModel, DeviceID, 0x00);
		sqlite3_finalize(stmt);
		return -1;
	}
	out = cJSON_Print(root);	//out中保存的是JSON格式的字符串
	//		dispatch_packet_to_clients()
	pthread_mutex_lock(&mutex);
	lgpe_Socket_Dispatch_To_Clients(g_server_clients_cmd, out, strlen(out));
	pthread_mutex_unlock(&mutex);
	cJSON_Delete(root);
	free(out);
//	 printf("%s\n",out);
	sqlite3_finalize(stmt);
	return 1;
}

int GetControlParam(cJSON *json)
{
//	char *zErrMsg = 0;
	char sql[256];
	int result;
	char *Cmd;
	char DeviceModel;
	char DeviceID;
	sqlite3_stmt *stmt;

	cJSON *root, *r, *fld;
	char *out;

	Cmd = cJSON_GetObjectItem(json, "Cmd")->valuestring;
	DeviceModel = cJSON_GetObjectItem(json, "DeviceModel")->valueint;
	DeviceID = cJSON_GetObjectItem(json, "DeviceID")->valueint;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Cmd", requstControlParamCmd);
	cJSON_AddNumberToObject(root, "DeviceModel", DeviceModel);
	cJSON_AddNumberToObject(root, "DeviceID", DeviceID);
	sprintf(sql, "select * from CtrParamTable");

	result = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
	if (SQLITE_OK == result)
	{
		cJSON_AddItemToObject(root, "DbResult", r = cJSON_CreateArray());

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{

			/*      	 fprintf(stdout,"%d  ,%d  ,%s  ,%s\n",
			 sqlite3_column_int(stmt,0),
			 sqlite3_column_int(stmt,1),
			 sqlite3_column_text(stmt,2),
			 sqlite3_column_text(stmt,3)
			 );
			 */
			cJSON_AddItemToArray(r, fld = cJSON_CreateObject());
			//     	 cJSON_AddStringToObject(fld,"Id",sqlite3_column_int(stmt,0));
			cJSON_AddNumberToObject(fld, "VbusRef",
					sqlite3_column_double(stmt, 1));
			cJSON_AddNumberToObject(fld, "VbusBase",
					sqlite3_column_double(stmt, 2));
			cJSON_AddNumberToObject(fld, "VcapRef",
					sqlite3_column_double(stmt, 3));
			cJSON_AddNumberToObject(fld, "VcapBase",
					sqlite3_column_double(stmt, 4));
			cJSON_AddNumberToObject(fld, "VbusKp",
					sqlite3_column_double(stmt, 5));
			cJSON_AddNumberToObject(fld, "VbusKi",
					sqlite3_column_double(stmt, 6));
			cJSON_AddNumberToObject(fld, "VcapKp",
					sqlite3_column_double(stmt, 7));
			cJSON_AddNumberToObject(fld, "VcapKi",
					sqlite3_column_double(stmt, 8));
			cJSON_AddNumberToObject(fld, "IdKp",
					sqlite3_column_double(stmt, 9));
			cJSON_AddNumberToObject(fld, "IdKi",
					sqlite3_column_double(stmt, 10));
			cJSON_AddNumberToObject(fld, "IqKp", sqlite3_column_int(stmt, 11));
			cJSON_AddNumberToObject(fld, "IqKi", sqlite3_column_int(stmt, 12));
		}
	}
	else
	{
		DbReturn(Cmd, DeviceModel, DeviceID, 0x00);
		sqlite3_finalize(stmt);
		return -1;
	}
	out = cJSON_Print(root);	//out中保存的是JSON格式的字符串
	//		dispatch_packet_to_clients()
	pthread_mutex_lock(&mutex);
	lgpe_Socket_Dispatch_To_Clients(g_server_clients_cmd, out, strlen(out));
	pthread_mutex_unlock(&mutex);
	cJSON_Delete(root);
	free(out);
//	 printf("%s\n",out);
	sqlite3_finalize(stmt);
	return 1;
}


void CreatePasswd_JSONStr(char *cmd,char *username,char *passwd,char **JSON_string)
{
    cJSON *JSON_object;//JSONN对象
    JSON_object=cJSON_CreateObject();
    cJSON_AddStringToObject(JSON_object,"Cmd",cmd);
    cJSON_AddNumberToObject(JSON_object,"DeviceModel",0x11);
    cJSON_AddNumberToObject(JSON_object,"DeviceID",0x00);
    cJSON_AddStringToObject(JSON_object,"UsrName",username);
    cJSON_AddStringToObject(JSON_object,"PassWord",passwd);
    *JSON_string =cJSON_Print(JSON_object);
    cJSON_Delete(JSON_object);
}


/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   	*
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _LGPE_SQLITE_C_ */
