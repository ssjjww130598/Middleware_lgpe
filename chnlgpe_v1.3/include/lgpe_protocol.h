/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_protocol.h
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_protocol.h
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  Chase  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _LGPE_PROTOCOL_H_
#define _LGPE_PROTOCOL_H_

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*----------------------------------------------------------*
 * 调试开关             Debug Switch Section                		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件             Include File Section                		*
 *----------------------------------------------------------*/
 
//#include"lgpe_header.h"
#include "dsplink.h"
#include "cJSON.h"

/*----------------------------------------------------------*
 * 全局宏定义           Global Macro Define Section         		*
 *----------------------------------------------------------*/


/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     		*
 *----------------------------------------------------------*/
/* JSON解析配置元素 */
typedef struct _configElem {
	Uint16 deviceModel;
	Uint16 packetType;
	cJSON *config;
	void* next;
} LGPEconfigElem;

/* JSON解析配置链表 */
typedef struct _configList {
	int length;
	LGPEconfigElem* head;
	LGPEconfigElem* tail;
} LGPEconfigList;
/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     		*
 *----------------------------------------------------------*/

extern LGPEconfigList *g_jsonConfigList;			/* JSON配置文件链表 */

/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    	*
 *----------------------------------------------------------*/
LGPEconfigList *LGPEConfig_create_list();								/*协议解析json格式配置链表创建*/
void LGPEConfig_remove_list(LGPEconfigList* clist);						/*协议解析json格式配置链表移除*/
void LGPEConfig_add_elem(LGPEconfigList* clist, LGPEconfigElem* elem);	/*协议解析json格式配置链表添加新元素*/
int LGPE_ConfigReadFromFile();											/*通过配置文件获取配置*/

unsigned char *ARM_AnalysisData(unsigned char *data);		/*解析运行数据*/
int ARM_AnalysisSocketData(unsigned char *sSocketFrame); /*解析socket数据*/
int ARM_packageCmd(unsigned char *chnlsendstr, cJSON *type,cJSON *cmd); /*封装数据流*/
void ARM_packageCmdReturn(unsigned char *str);/*命令反馈json封装*/
unsigned char *ARM_packageSystemRunStop(int type,cJSON* pJson);/*解析socket封装成启停命令数据流*/
void *lgpe_protocol_datain_handler();	/*DSP/Link接收处理线程*/
/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   	*
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _LGPE_PROTOCOL_H_ */
