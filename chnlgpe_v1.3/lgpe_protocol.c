/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_protocol.c
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_protocol.c
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  Chase  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 调试开关           Debug Switch Section                  		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件           Include File Section                  		*
 *----------------------------------------------------------*/
/*  ----------Application Header-------------*/
#include "lgpe_header.h"
/*----------------------------------------------------------*
 * 全局变量定义       Global Variable Define Section        	*
 *----------------------------------------------------------*/
Uint16 g_deviceModel = 0x11;				/* 设备型号 */
Uint8 g_deviceId = 0;						/* 设备ID */
cJSON *g_jsonOut;							/* dsplink输出JSON结构体 */
cJSON *g_jsonCmdConfig;						/*命令ID对应表JSON结构体*/
LGPEconfigList *g_jsonConfigList;			/* JSON配置文件链表 */
extern pthread_mutex_t mutex;
extern pthread_mutex_t sql_mutex;
extern sem_t sem;
extern LGPEsqloperateList* g_sqlOperate_list;
/*----------------------------------------------------------*
 * 局部宏定义         Local Macro Define Section            		*
 *----------------------------------------------------------*/
#define DIRNAME "json_config"
#define OLD 0
//#define MTRACE_ENABLE
/*----------------------------------------------------------*
 * 局部结构定义       Local Structure Define Section        	*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 局部函数原型声明   Local Prototype Declare Section       	*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 静态变量定义       Static Variable Define Section        	*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 函数定义           Function Define Section               		*
 *----------------------------------------------------------*/


/*协议解析json格式配置链表创建*/
LGPEconfigList *LGPEConfig_create_list()
{
	LGPEconfigList *list = (LGPEconfigList*)malloc(sizeof(LGPEconfigList));
	list->length=0;
	list->head=NULL;
	list->tail=NULL;
	return list;
}

/*协议解析json格式配置链表移除*/
void LGPEConfig_remove_list(LGPEconfigList* clist)
{
	LGPEconfigElem* elem;
	LGPEconfigElem* temp;
	elem = clist->head;
	while(elem != NULL)
	{
		temp = elem->next;
		cJSON_Delete(elem->config);
		free(elem);
		elem = temp;
	}
	free(clist);
}

/*协议解析json格式配置链表添加新元素*/
void LGPEConfig_add_elem(LGPEconfigList* clist, LGPEconfigElem* elem)
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

int LGPE_ConfigReadFromFile()
{

	/*readdir*/
//		unsigned int count=0;                               //临时计数，[0，SINGLENUM]
//	    char txtname[128];                                  //存放文本文件名
//	    FILE *fp;
	DIR *dp;
	struct dirent *dirp;
	cJSON *pJsonConfig;
	cJSON *pJsonArrayItem;
	cJSON *pJsonItem;
	char *str;
	char namestr[70];
	int namesize, time = 0;
//	int arraysize, i;
//	int namesize,time;
	g_jsonConfigList = LGPEConfig_create_list();
// CHNLGPE_0Print ("jsonconfiglist created\n") ;
	if ((dp = opendir(DIRNAME)) == NULL)
	{
		perror("LGPE_ConfigReadFromFile: opendir");
		return -1;
	}
	//开始遍历目录
	while ((dirp = readdir(dp)) != NULL)
	{
		//跳过'.'和'..'两个目录
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;

		namesize = strlen(dirp->d_name);

		//如果是.json文件，长度至少是6
		if (namesize < 6)
			continue;

		//只存取.json扩展名的文件名
		if (strcmp((dirp->d_name + (namesize - 5)), ".json") != 0)
			continue;
		time++;
		/*从配置文件获取json结构体*/
		strcpy(namestr, DIRNAME);
		strcat(namestr, "/");
		strcat(namestr, dirp->d_name);
		pJsonConfig = GetJsonObjectFormfile(namestr);
		if (pJsonConfig)
		{
			/*main json获取deviceType和公司名等*/
			if (!strcmp(dirp->d_name, "svg_main_config.json"))
			{
				pJsonArrayItem = cJSON_GetArrayItem(pJsonConfig, 0);
				if (!pJsonItem)
				{
					LGPEConfig_remove_list(g_jsonConfigList);
					cJSON_Delete(pJsonConfig);
					CHNLGPE_0Print("no array[0]\n");
					return -1;
				}
				else
				{
					pJsonItem = cJSON_GetObjectItem(pJsonArrayItem,
							"deviceModel");
					str = pJsonItem->valuestring;
					g_deviceModel = str2int(str);

//					CHNLGPE_1Print("main config g_deviceModel: %d deviceName: %s\n",g_deviceModel,cJSON_GetObjectItem(pJsonArrayItem,"deviceName")->valuestring);
					CHNLGPE_1Print("LGPE_ConfigReadFromFile: company: %s\n",
							(Uint32)cJSON_GetObjectItem(pJsonArrayItem, "company")->valuestring);
				}
				continue;
			}
			/*cmd_configjson获取命令对应包ID*/
			else if (!strcmp(dirp->d_name, "svg_socket_cmd_config.json"))
			{
				g_jsonCmdConfig = pJsonConfig;
				continue;
			}
			/*读取其他文件的json结构体*/
			if (!pJsonConfig)
			{
				LGPEConfig_remove_list(g_jsonConfigList);
				CHNLGPE_0Print("LGPE_ConfigReadFromFile: json configuration file error\n");
				cJSON_Delete(pJsonConfig);
				return -1;
			}
			else
			{
				LGPEconfigElem * elem = (LGPEconfigElem *) malloc(
						sizeof(LGPEconfigElem));
				elem->config = pJsonConfig;
				pJsonArrayItem = cJSON_GetArrayItem(pJsonConfig, 0);
				if (!pJsonArrayItem)
				{
					LGPEConfig_remove_list(g_jsonConfigList);
					CHNLGPE_0Print("LGPE_ConfigReadFromFile: json format error: no array[0]\n");
					cJSON_Delete(pJsonConfig);
					return -1;
				}
				else
				{
					pJsonItem = cJSON_GetObjectItem(pJsonArrayItem,
							"deviceModel");
					if (!pJsonItem)
					{
						LGPEConfig_remove_list(g_jsonConfigList);
						cJSON_Delete(pJsonConfig);
						CHNLGPE_0Print("LGPE_ConfigReadFromFile: json format error: no deviceModel\n");
						return -1;
					}
					else
					{
						str = pJsonItem->valuestring;
						elem->deviceModel = str2int(str);
					}
					pJsonItem = cJSON_GetObjectItem(pJsonArrayItem, "packetID");
					if (!pJsonItem)
					{
						LGPEConfig_remove_list(g_jsonConfigList);
						cJSON_Delete(pJsonConfig);
						CHNLGPE_0Print("LGPE_ConfigReadFromFile: json format error: no packetID\n");
						return -1;
					}
					else
					{
						str = pJsonItem->valuestring;
						elem->packetType = str2int(str);
					}
				}
				LGPEConfig_add_elem(g_jsonConfigList, elem);
			}
		}
	}
	CHNLGPE_1Print("LGPE_ConfigReadFromFile: read json file: %dtimes\n", time);
	return 0;
}

/*获取配置文件CSJON指针*/
cJSON *getJsonConfig(Uint16 deviceModel,Uint16 packetId)
{
	if (g_jsonConfigList)
	{
		LGPEconfigElem *elem = g_jsonConfigList->head;
		//printf("devicemodel: %d,packetID: %d\n",elem->deviceModel,elem->packetType);
		while(elem)
		{
			if(elem->deviceModel == deviceModel && elem->packetType == packetId)
			{
//				printf("device ok\n");
				return elem->config;
			}
			elem = elem->next;
		}
	}
	return NULL;
}
#if OLD
/* DSP/link 数据解析函数*/
unsigned char *ARM_AnalysisData(unsigned char *data)
{

	/*用于控制只打印一次*/
#ifdef MTRACE_ENABLE
	static int flag = 0;
	/*跟踪内存分配和回收*/
	if (flag == 0)
		mtrace();
#endif
	unsigned char *str;
	cJSON *configLocal = getJsonConfig(data[4], data[6]);

//	cJSON_Delete(m_jsonOut);


	if (!configLocal)
	{
		CHNLGPE_0Print("ARM_analysis: No config file found!\n");
		return NULL;
	}
	else if (data[4] != g_deviceModel)
	{
		CHNLGPE_0Print("ARM_analysis: deviceModel error\n");
		return NULL;
	}
	else
	{
		/*非运行数据解析如命令反馈*/
		if (!(data[6] == 0x31))
		{
			return NULL;
		}
		/*运行数据解析*/
		else
		{
			g_jsonOut = cJSON_CreateArray();
			cJSON *c = cJSON_GetArrayItem(configLocal, 1);
			cJSON *n;
			while (c)
			{
				cJSON *type = cJSON_GetObjectItem(c, "type");
				cJSON *name = cJSON_GetObjectItem(c, "name");
				int offset = atoi(cJSON_GetObjectItem(c, "byteOffset")->valuestring);
//				lgdebug(DBG_2, "ARM_analysis: type:%s; name:%s; offset:%d;\n",type->valuestring, name->valuestring, offset);
				/*Uint8类型解析*/
				if (!strcmp(type->valuestring,"Uint8"))
				{
					n = cJSON_CreateObject();
					cJSON_AddNumberToObject(n, name->valuestring, data[offset]);
					cJSON_AddItemToArray(g_jsonOut, n);
				}
				/*Uint16类型解析*/
				else if (!strcmp(type->valuestring, "Uint16"))
				{
					n = cJSON_CreateObject();
					cJSON_AddNumberToObject(n, name->valuestring, AfxDataToInt16(&data[offset]));
					cJSON_AddItemToArray(g_jsonOut, n);
				}
				/*Uint32类型解析*/
				else if (!strcmp(type->valuestring, "Uint32"))
				{
					n = cJSON_CreateObject();
					cJSON_AddNumberToObject(n, name->valuestring, AFXDataToInt(&data[offset]));
					cJSON_AddItemToArray(g_jsonOut, n);
				}
				/*float类型解析*/
				else if (!strcmp(type->valuestring, "float"))
				{
					n = cJSON_CreateObject();
					cJSON_AddNumberToObject(n, name->valuestring, AfxDataToFloat(&data[offset]));
					cJSON_AddItemToArray(g_jsonOut, n);
				}
				/*下一个*/
				c = c->next;
			}
			str = (unsigned char*)cJSON_Print(g_jsonOut);
//		lgdebug(DBG_WARNING,"Data strem from DSP: %s.\n",str);
			/*if (!flag)
			{
				printf("string length: %d\n", strlen(str));
				printf("Data stream from DSP: %s.\n", str);
				flag = 1;
			}*/
//		free(str);
//			CHNLGPE_0Print("before delete\n");
			cJSON_Delete(g_jsonOut);
#ifdef MTRACE_ENABLE
			if (flag == 0)
			{
				muntrace();
				flag = 1;
			}
#endif
			return str;
		}
	}
}
#else
/* DSP/link 数据解析函数*/
unsigned char *ARM_AnalysisData(unsigned char *data)
{

	/*用于控制只打印一次*/
#ifdef MTRACE_ENABLE
	static int flag = 0;
	/*跟踪内存分配和回收*/
	if (flag == 0)
		mtrace();
#endif
	unsigned char *str;
	cJSON *configLocal = getJsonConfig(data[4], data[6]);

//	cJSON_Delete(m_jsonOut);


	if (!configLocal)
	{
		CHNLGPE_0Print("ARM_analysis: No config file found!\n");
		return NULL;
	}
	else if (data[4] != g_deviceModel)
	{
		CHNLGPE_0Print("ARM_analysis: deviceModel error\n");
		return NULL;
	}
	else
	{
//		/*非运行数据解析如命令反馈*/
//		if (data[6] != 0x31)
//		{
//			g_jsonOut = cJSON_CreateObject();
//			cJSON *c = cJSON_GetArrayItem(configLocal, 1);
//			cJSON *n;
//			whlie(c)
//
//
//			return NULL;
//		}
		/*数据解析*/
//		if(data[6] == 0x31))
//		{
			g_jsonOut = cJSON_CreateObject();
			cJSON *c = cJSON_GetArrayItem(configLocal, 1);
			cJSON *n;
			while (c)
			{
				cJSON *type = cJSON_GetObjectItem(c, "type");
				cJSON *name = cJSON_GetObjectItem(c, "name");
				int offset = atoi(cJSON_GetObjectItem(c, "byteOffset")->valuestring);
//				lgdebug(DBG_2, "ARM_analysis: type:%s; name:%s; offset:%d;\n",type->valuestring, name->valuestring, offset);
				/*Uint8类型解析*/
				if (!strcmp(type->valuestring,"Uint8"))
				{
					cJSON_AddNumberToObject(g_jsonOut,name->valuestring,data[offset]);
				}
				/*Uint16类型解析*/
				else if (!strcmp(type->valuestring, "Uint16"))
				{
					cJSON_AddNumberToObject(g_jsonOut,name->valuestring,AfxDataToInt16(&data[offset]));
				}
				/*Uint32类型解析*/
				else if (!strcmp(type->valuestring, "Uint32"))
				{
					cJSON_AddNumberToObject(g_jsonOut,name->valuestring,AFXDataToInt(&data[offset]));
				}
				/*float类型解析*/
				else if (!strcmp(type->valuestring, "float"))
				{
					cJSON_AddNumberToObject(g_jsonOut,name->valuestring,AfxDataToFloat(&data[offset]));
				}
				/*下一个*/
				c = c->next;
			}
			str = (unsigned char*)cJSON_Print(g_jsonOut);
//		lgdebug(DBG_WARNING,"Data strem from DSP: %s.\n",str);
			/*if (!flag)
			{
				printf("string length: %d\n", strlen(str));
				printf("Data stream from DSP: %s.\n", str);
				flag = 1;
			}*/
//		free(str);
//			CHNLGPE_0Print("before delete\n");
			cJSON_Delete(g_jsonOut);
//			printf("Data stream from DSP: %s.\n", str);
#ifdef MTRACE_ENABLE
			if (flag == 0)
			{
				muntrace();
				flag = 1;
			}
#endif
			return str;
//		}
	}
}
#endif
/* socket 数据解析函数*/
int ARM_AnalysisSocketData(unsigned char *sSocketFrame)
{
	cJSON *pJson, *pJsonItem;
//	unsigned char *str;
	unsigned char *chnsendstr;
//	char str_print[1024],str_tmp[100];
	int i;
//	chnsendstr = malloc(1024);
	printf("ARM_AnalysisSocketData: %s\n",(char *)sSocketFrame);
	pJson = cJSON_Parse((char *)sSocketFrame);
	if (!pJson)
	{
		/*json格式不正确*/
		CHNLGPE_0Print("ARM_AnalysisSocketData:malformed data received from socket\n");
		return -1;
	}
	/*是否有Devicemodel项*/
	if (!(pJsonItem = cJSON_GetObjectItem(pJson, "DeviceModel")))
	{
		/*json格式不正确*/
		CHNLGPE_0Print("ARM_AnalysisSocketData:malformed data received from socket\n");
		return -1;
	}
	else
	{
		/*自动判断数值类型*/
		/*int类型*/
		if (pJsonItem->type == cJSON_Number)
		{
			if (pJsonItem->valueint != g_deviceModel)
			{
				/*设备类型不匹配*/
				CHNLGPE_0Print("ARM_AnalysisSocketData:DeviceModel mismatch\n");
				return -1;
			}
		}
		/*字符串类型*/
		else if (pJsonItem->type == cJSON_String)
		{
			if (str2int(pJsonItem->valuestring) != g_deviceModel)
			{
				/*设备类型不匹配*/
				CHNLGPE_0Print("ARM_AnalysisSocketData:DeviceModel mismatch\n");
				return -1;
			}
		}
		/*其他类型*/
		else
		{
			/*json格式不正确*/
			CHNLGPE_0Print("ARM_AnalysisSocketData:malformed data received from socket\n");
			return -1;
		}
	}
	if (!(pJsonItem = cJSON_GetObjectItem(pJson, "Cmd")))
	{
		/*json格式不正确*/
		CHNLGPE_0Print("ARM_AnalysisSocketData:malformed data received from socket\n");
		return -1;
	}
	cJSON *c = cJSON_GetObjectItem(g_jsonCmdConfig, pJsonItem->valuestring);
	if (!c)
	{
		/*json配置文件中没有cmd对应的packetID*/
		CHNLGPE_0Print("ARM_AnalysisSocketData:this cmd hasn't a matched packetID\n");
		return 1;
	}
	else
	{
		/*获取数据帧*/
		chnsendstr = malloc(CHNL_PACKET_LEN);
		ARM_packageCmd(chnsendstr, c, pJson);
		/*发送数据帧*/
		chnlSendCmdFrame(chnsendstr, AfxDataToInt16(chnsendstr));

		printf("ARM_AnalysisSocketData: Cmd Packet Dump (%dbytes): ", AfxDataToInt16(chnsendstr));
		for(i = 0; i < 20; i++)
		{
			printf("%x ", chnsendstr[i]);
		}
		printf("\n");
		fflush (stdout);
//		sprintf(str_print,"Cmd Packet Dump: ");
//		for(i = 0; i < AfxDataToInt16(chnsendstr); i++)
//		{
//			sprintf(str_tmp,"%x ", chnsendstr[i]);
//			strcat(str_print,str_tmp);
//		}
//		lgdebug(DBG_WARNING, str_print);

		free(chnsendstr);
	}
	return 0;
}
void ARM_packageCmdReturn(unsigned char *str)
{
	cJSON *pJson,*pJsonOut;
	pJsonOut = cJSON_CreateObject();
	pJson = cJSON_Parse(str);
	free(str);
	/*开始封装*/


	/*输出字符串*/
	str = cJSON_Print(pJsonOut);

	return;
}
/*数据流封装*/
int ARM_packageCmd(unsigned char *chnlsendstr, cJSON *type,cJSON *cmd)
{
	Uint16 i = 2;
	Uint16 packetId = 0;
	char *mode;
	int deviceID;
	char *status;
	memset(chnlsendstr, 0, CHNL_PACKET_LEN);

	/*添加包类型，接收发送地址*/
	chnlsendstr[i++] = ADDR_GUARD_DSP;
	chnlsendstr[i++] = ADDR_GUARD_ARM;
	chnlsendstr[i++] = g_deviceModel;
	chnlsendstr[i++] = g_deviceId;

	/*获取packetType*/
	if (type->type == cJSON_Number)
		packetId = type->valueint;
	else if(type->type == cJSON_String)
		packetId = str2int(type->valuestring);
	cJSON *configLocal = getJsonConfig(g_deviceModel, packetId);
	/*将PacketType添加到cmd中，之后直接从JSON中检索，生成数据包*/
	cJSON_AddNumberToObject(cmd, "PacketType", packetId);
	switch(packetId)
	{
	/*运行模式切换*/
	case HEX_CMD_WORK_PATTERN:
		mode = cJSON_GetObjectItem(cmd, "Mode")->valuestring;
		if(!strcmp(mode, "Debug_Low"))	cJSON_AddNumberToObject(cmd, "Command", HEX_DEBUG_LOW);
		if(!strcmp(mode, "Debug_High"))	cJSON_AddNumberToObject(cmd, "Command", HEX_DEBUG_HIGH);
		if(!strcmp(mode, "Running"))	cJSON_AddNumberToObject(cmd, "Command", HEX_RUNNING);
		break;
	/*系统启停*/
	case HEX_CMD_START_STOP:
		mode = cJSON_GetObjectItem(cmd, "Mode")->valuestring;
		if(!strcmp(mode, "Start"))	cJSON_AddNumberToObject(cmd, "Command", HEX_START);
		if(!strcmp(mode, "Stop"))	cJSON_AddNumberToObject(cmd, "Command", HEX_STOP);
		if(!strcmp(mode, "Em_Stop"))	cJSON_AddNumberToObject(cmd, "Command", HEX_EM_STOP);
		break;
	/*复归*/
	case HEX_CMD_RESET:
		cJSON_AddNumberToObject(cmd, "Command", HEX_RESET);
		break;
	/*断路器操作*/
	case HEX_CMD_BREAKER_OPERATION:
		deviceID = cJSON_GetObjectItem(cmd, "Device_id")->valueint;
		cJSON_AddNumberToObject(cmd, "ControlID", deviceID);
		status = cJSON_GetObjectItem(cmd, "Status")->valuestring;
		if(!strcmp(status, "On")) 	cJSON_AddNumberToObject(cmd, "Command", HEX_ACTION_ON);
		if(!strcmp(status, "Off")) 	cJSON_AddNumberToObject(cmd, "Command", HEX_ACTION_OFF);
		break;
	/*参数设置*/
	case HEX_CMD_SET_PROJECT_PARAM:
	case HEX_CMD_SET_GUARD_PARAM:
	case HEX_CMD_SET_CONTROL_PARAM:
	case HEX_CMD_SET_SAMPLE_PARAM:
		break;
	}

	/*添加时间戳*/
	Timestamp *time_now = timestamp_new();
	lgpe_get_localtime(time_now);
	cJSON_AddNumberToObject(cmd, "Year", (time_now->time->tm_year + 1900));
	cJSON_AddNumberToObject(cmd, "Month", (time_now->time->tm_mon + 1));
	cJSON_AddNumberToObject(cmd, "Day", time_now->time->tm_mday);
	cJSON_AddNumberToObject(cmd, "Hour", time_now->time->tm_hour);
	cJSON_AddNumberToObject(cmd, "Minute", time_now->time->tm_min);
	cJSON_AddNumberToObject(cmd, "Second", time_now->time->tm_sec);

	timestamp_delete(time_now);

	/*通过JSON配置文件生成数据包*/
	cJSON *c = cJSON_GetArrayItem(configLocal, 1);
	while(c)
	{
		char *name = cJSON_GetObjectItem(c, "name")->valuestring;
		cJSON *byteoffset = cJSON_GetObjectItem(c, "byteOffset");
		if(byteoffset->type == cJSON_Number)
			i = byteoffset->valueint;
		if(byteoffset->type == cJSON_String)
			i = str2int(byteoffset->valuestring);

		/*CheckSum表示遇到结尾*/
		if(!strcmp(name, "CheckSum") || i >= CHNL_PACKET_LEN - 5)
		{
			break;
		}
		char *type = cJSON_GetObjectItem(c, "type")->valuestring;
		cJSON *n = cJSON_GetObjectItem(cmd,name);
		if(n)
		{
			if(!strcmp(type, "Uint8"))	chnlsendstr[i] = (Uint8)n->valueint;
			if(!strcmp(type, "Uint16"))	AfxInt16ToData(&chnlsendstr[i], (Uint16)n->valueint);
			if(!strcmp(type, "float"))	AfxFloatToData(&chnlsendstr[i], (float)n->valuedouble);
		}

		lgdebug(DBG_1,"ARM_packageCmd: Packaging [%d],type: %s, %s\n",i,type,cJSON_Print(c));
		c = c->next;
	}

	/*数据包长度*/
	AfxInt16ToData(chnlsendstr, (i + 1));
	/*校验和*/
	chnlsendstr[i] = MakeSum(chnlsendstr, (i + 1));

	return 0;
}
/*不用此函数*/
unsigned char *ARM_packageSystemRunStop(int type, cJSON * pJson)
{
	cJSON* fmt;
	unsigned char *chnsendstr;
	chnsendstr = malloc(1024);
	/*封装数据帧*/
	/*长度*/
	AfxInt16ToData(chnsendstr,16);
	chnsendstr += 2;
	/*接受地址*/
	*chnsendstr++ = ADDR_GUARD_DSP;
	/*发送地址*/
	*chnsendstr++ = ADDR_GUARD_ARM;
	/*设备型号*/
	*chnsendstr++ = g_deviceModel;
	/*设备ID*/
	/*是否有DeviceID项*/
	if (!(fmt = cJSON_GetObjectItem(pJson, "DeviceID")))
	{
		/*json格式不正确*/
		CHNLGPE_0Print("malformed data received from socket\n");
		free(chnsendstr);
		return NULL;
	}
	else
	{
		/*int类型*/
		if (fmt->type == 3)
		{
			*chnsendstr++ = fmt->valueint;
		}
		/*字符串类型*/
		else if (fmt->type == 4)
		{
			*chnsendstr++ = str2int(fmt->valuestring);
		}
		/*其他类型*/
		else
		{
			/*json格式不正确*/
			CHNLGPE_0Print("malformed data received from socket\n");
			free(chnsendstr);
			return NULL;
		}
	}
	/*包类型*/
	*chnsendstr++ = HEX_CMD_START_STOP;
	/*数据*/
	*chnsendstr++ = HEX_START;
	/*时间戳*/
	*chnsendstr++ = 0;
	*chnsendstr++ = 0;
	*chnsendstr++ = 0;
	*chnsendstr++ = 0;
	*chnsendstr++ = 0;
	*chnsendstr++ = 0;
	*chnsendstr++ = 0;
	/*压入发送队列*/
	*chnsendstr = MakeSum(chnsendstr - 15, 15);
	return NULL;
}

/*DSP/Link接收处理线程*/
void *lgpe_protocol_datain_handler()
{
	unsigned char *strOut = NULL;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);	/*线程可被结束*/
	for(;;)
	{
//		CHNLGPE_0Print("lgpe_protocol_datain_handler: sem_waiting.\n");
//		sleep(10);
		if (g_dsplinkExit == 1)
		{
			return NULL;
		}

//		lgdebug(DBG_1,"lgpe_protocol_datain_handler:g_dsplink_shm - %x,%x;\n", (Uint32)g_dsplink_shm, (Uint32)&g_dsplink_shm->dsplink_rx_sem);
		sem_wait(&g_dsplink_shm->dsplink_rx_sem);
		lgdebug(DBG_1,"lgpe_protocol_datain_handler: Handling DSP/Link data in with length %d bytes.\n", g_dsplink_shm->rx_length);
		strOut = ARM_AnalysisData(g_dsplink_shm->rx_buffer);
		if (strOut)
		{
			if(g_dsplink_shm->rx_buffer[6] == 0xff)
				ARM_packageCmdReturn(strOut);
			/*Socket端口分发数据*/
			lgdebug(DBG_1,"lgpe_protocol_datain_handler: dispatching data to socket with length %d bytes.\n",(int)strlen((char *)strOut));
			pthread_mutex_lock(&mutex);
			lgpe_Socket_Dispatch_To_Clients(g_server_clients_data,strOut, (int)strlen((char *)strOut));
			pthread_mutex_unlock(&mutex);
			free(strOut);
		}
	}
}
