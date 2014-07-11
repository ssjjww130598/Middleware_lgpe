/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_utility.h
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_utility.h
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  Chase  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _LGPE_UTILITY_H_
#define _LGPE_UTILITY_H_

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*----------------------------------------------------------*
 * 调试开关             Debug Switch Section                		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件             Include File Section                		*
 *----------------------------------------------------------*/
//#include "lgpe_header.h"
#include "dsplink.h"
/*----------------------------------------------------------*
 * 全局宏定义           Global Macro Define Section         		*
 *----------------------------------------------------------*/


/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     		*
 *----------------------------------------------------------*/
enum
{
 DBG_1,
 DBG_2,
 DBG_3,
 DBG_4,
 DBG_5,
 DBG_INFO,
 DBG_WARNING,
 DBG_ERROR,
 DBG_OFF
};

typedef struct _timestamp {

  struct tm * time;
}Timestamp;

/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     		*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    	*
 *----------------------------------------------------------*/
void lgdebug_set_level_s(char* level);
void lgdebug_set_level(int level);
int lgdebug_get_level();
void lgdebug(int debug_level,char* fmt, ...);
void lgdebug_nl(int debug_level,char* fmt, ...);
void lgdebug_raw(int debug_level,char* fmt, ...);
void lgdebug_initialize(char* filename);

Timestamp *timestamp_new (void);			/*新建时间戳结构体*/
void timestamp_delete (Timestamp * ts);		/*回收时间戳结构体*/
void lgpe_get_localtime(Timestamp *ts);		/*获取当前时间戳*/

/** ============================================================================
 *  @func   atoi
 *
 *  @desc   Extern declaration for function that converts a string into an
 *          integer value.
 *
 *  @arg    str
 *              String representation of the number.
 *
 *  @ret    <valid integer>
 *              If the 'initial' part of the string represents a valid integer
 *          0
 *              If the string passed does not represent an integer or is NULL.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */
extern int atoi (const char * str) ;


/** ============================================================================
 *  @func   CHNLGPE_Atoi
 *
 *  @desc   This function converts a string into an integer value.
 *
 *  @arg    str
 *              String representation of the number.
 *
 *  @ret    <valid integer>
 *              If the 'initial' part of the string represents a valid integer
 *          0
 *              If the string passed does not represent an integer or is NULL.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */
#define CHNLGPE_Atoi atoi


/** ============================================================================
 *  @func   CHNLGPE_Atoll
 *
 *  @desc   Converts ascii to long int
 *
 *  @ret    <valid integer>
 *              If the 'initial' part of the string represents a valid integer
 *          0
 *              If the string passed does not represent an integer or is NULL.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */
NORMAL_API
Uint32
CHNLGPE_Atoll (Char8 * str) ;

/** ============================================================================
 *  @func   CHNLGPE_0Print
 *
 *  @desc   Print a message without any arguments.
 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
Void
CHNLGPE_0Print (Char8 * str);

/** ============================================================================
 *  @func   CHNLGPE_1Print
 *
 *  @desc   Print a message with one arguments.
 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
Void
CHNLGPE_1Print (Char8 * str, Uint32 arg);

void AfxInt16ToData(unsigned char *pBuf, Uint16 Data);//int16转字符
Uint16 AfxDataToInt16(unsigned char *pBuf);//字符转int16
void AfxIntToData(unsigned char *pBuf, int Data);//int转字符
int AFXDataToInt(unsigned char *pBuf);//字符转int
void AfxFloatToData(unsigned char *pBuf, float Data);//float转字符
float AfxDataToFloat(unsigned char *pBuf);//字符转float
char	CharToHexChar(unsigned char ch);//ascii转16进制
unsigned char writeBitTrue(unsigned char c,int pos);			//写字节指定位数为1
unsigned char writeBitFalse(unsigned char c,int pos);			//写字节指定位数为0
unsigned int  str2int(char *str);		/*字符串转int函数*/

Uint8 MakeSum(Uint8 *buf,Uint16 len);		/*校验和计算*/
int xassert_fatal(const char *msg, int result);

cJSON* GetJsonObjectFormfile(char* fileName);
/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   	*
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _LGPE_UTILITY_H_ */
