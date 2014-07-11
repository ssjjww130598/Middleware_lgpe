/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_utility.c
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_utility.c
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
/*----------Application Header-------------*/
#include "lgpe_header.h"
/*----------------------------------------------------------*
 * 全局变量定义       Global Variable Define Section        	*
 *----------------------------------------------------------*/
int g_debug_level = DBG_1;
FILE* g_logfile = NULL;
/*----------------------------------------------------------*
 * 局部宏定义         Local Macro Define Section            		*
 *----------------------------------------------------------*/
#define TIMESTRING_SIZE 200
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
CHNLGPE_0Print (Char8 * str)
{
    printf (str) ;
    fflush (stdout) ;
}


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
CHNLGPE_1Print (Char8 * str, Uint32 arg)
{
    printf (str, arg) ;
    fflush (stdout) ;
}


/** ============================================================================
 *  @func   CHNLGPE_Atoll
 *
 *  @desc   Converts ascii to long int
 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
Uint32
CHNLGPE_Atoll (Char8 * str)
{
     Uint32 val = 0 ;
     val = strtoll (str, NULL, 16) ;
     return val ;
}

/*新建时间戳结构体*/
Timestamp *timestamp_new (void) {

  Timestamp * ts = (Timestamp*)malloc(sizeof(Timestamp));
  // Shred memory to indicate initialization status.
  memset((void*)ts,0xda,sizeof(Timestamp));
  return ts;
}
/*回收时间戳结构体*/
void timestamp_delete (Timestamp * ts) {

  // Shred memory going out to indicate invalid access.
  memset((void*)ts,0xdd,sizeof(Timestamp));
  free(ts);
}


/**
 * Return string of timestamp in YYYY/MM/DD hh:mm:ss format.
 *
 * @version   2004/9/27     mturon     Initial revision
 */
void
timestamp_get_string (Timestamp * ts, char timestring[TIMESTRING_SIZE]) {

  struct tm * l_time = ts->time;
  time_t timetype;

  timetype = time(NULL);
  l_time = localtime(&timetype);

  strftime(timestring, TIMESTRING_SIZE, "%Y/%m/%d %H:%M:%S", l_time);
}


char g_timestring[TIMESTRING_SIZE];
char* lgprint_get_timestamp(){
	Timestamp *time_now;
	memset(g_timestring,0,TIMESTRING_SIZE);
	time_now = timestamp_new();
	timestamp_get_string(time_now, g_timestring);
	timestamp_delete(time_now);
	return g_timestring;
}

/*
 * @func lgpe_get_localtime
 *
 * @desc 获取当前时间戳
 *
 * @args
 *
 * @version		2014/03/21	 Chase	Initial	Version
 *
 */
void lgpe_get_localtime(Timestamp *ts)
{
	if(ts)
	{
		time_t timetype;

		timetype = time(NULL);
		ts->time = localtime(&timetype);
	}
	return;
}


void lgdebug_initialize(char* filename){

	// char* logfilename = (char*) xparams_get_param(LGPE_LOG_DIR_VAR,"./");
//	char* logfilename = "./log";
	char* logfilename = ".";
	char af[100];
	memset(af,0,100);

	if(logfilename == NULL){
		lgdebug(DBG_ERROR,"lgdebug: No log file specified \n");
		return;
	}

	strcpy(af,logfilename);
	strcat(af,"/");
	strcat(af,filename);

	//open log file
	g_logfile = fopen(af, "w");
	if(g_logfile == NULL){
		lgdebug(DBG_ERROR,"lgdebug: could not open log file %s: %s\n", af, strerror(errno));
		return;
	}
}

void lgdebug_set_level_s(char* level){
  if (strcasecmp(level, "DBG_OFF") == 0){
	g_debug_level = DBG_OFF;
  }else if (strcasecmp(level, "DBG_ERROR") == 0){
	g_debug_level = DBG_ERROR;
  }else if (strcasecmp(level, "DBG_WARNING") == 0){
	g_debug_level = DBG_WARNING;
  }else if (strcasecmp(level, "DBG_INFO") == 0){
	g_debug_level = DBG_INFO;
  }else if (strcasecmp(level, "DBG_1") == 0){
	g_debug_level = DBG_1;
  }else if (strcasecmp(level, "DBG_2") == 0){
	g_debug_level = DBG_2;
  }else if (strcasecmp(level, "DBG_3") == 0){
	g_debug_level = DBG_3;
  }else if (strcasecmp(level, "DBG_4") == 0){
	g_debug_level = DBG_4;
  }else if (strcasecmp(level, "DBG_5") == 0){
	g_debug_level = DBG_5;
  }
}

void lgdebug_set_level(int level){
	g_debug_level = level;
}

int lgdebug_get_level(){
	return g_debug_level;
}


void lgdebug(int debug_level, char* fmt, ...){

//	LGPEParams* xparams = xparams_get_lgpeparams();

 	//char* timestring = xprint_get_timestamp();
 	if(debug_level >= g_debug_level){
		char* timestring = lgprint_get_timestamp();
 		va_list argp;
		va_start(argp,fmt);

		if(g_logfile != NULL){
			fprintf(g_logfile,"[%s] ",timestring);
			vfprintf(g_logfile,fmt,argp);
			fflush(g_logfile);
		}
		va_end(argp);
	}
}

void lgdebug_raw(int debug_level, char* fmt, ...){

	//LGPEParams* xparams = xparams_get_lgpeparams();

 	//char* timestring = xprint_get_timestamp();
 	if(debug_level >= g_debug_level){
		va_list argp;
		va_start(argp,fmt);
		if(g_logfile != NULL){
			vfprintf(g_logfile,fmt,argp);
			fflush(g_logfile);
		}
		va_end(argp);
	}
}


void lgdebug_dump(int debug_level, char* buf, Uint32 count){

 	if(debug_level >= g_debug_level){
		char* timestring = lgprint_get_timestamp();
		Uint32 i;

		if(g_logfile != NULL){
			fprintf(g_logfile,"[%s] DataDump Started(%d):",timestring,(int)count);
			for(i=0;i<count;i++){
				if(i%10 == 0)
					fprintf(g_logfile,"\n");
				fprintf(g_logfile,"%02xH ",buf[i]);
			}
//			fprintf(g_logfile,"             %02xH ",g_sRunHarmData.RunData.cRevId);
//			fprintf(g_logfile,"             %02xH ",g_sRunHarmData.RunData.cSendId);
			fprintf(g_logfile,"\n");
			fflush(g_logfile);
		}
	}
}

void AfxInt16ToData(unsigned char *pBuf, Uint16 Data)
{
	Int16_Data int16Data;
	int16Data.int16_data = Data;
	pBuf[0] = int16Data.Hex_data[0];
	pBuf[1] = int16Data.Hex_data[1];
}
Uint16 AfxDataToInt16(unsigned char *pBuf)
{
	Int16_Data int16Data;
	int16Data.Hex_data[0] = pBuf[0];
	int16Data.Hex_data[1] = pBuf[1];
	return int16Data.int16_data;
}
//INT转字符
void AfxIntToData(unsigned char *pBuf, int Data)
{
	Int_Data intData;
	intData.int_data = Data;
	pBuf[0] = intData.Hex_data[0];
	pBuf[1] = intData.Hex_data[1];
	pBuf[2] = intData.Hex_data[2];
	pBuf[3] = intData.Hex_data[3];
}
//字符转INT
int AFXDataToInt(unsigned char *pBuf)
{
	Int_Data intData;
	intData.Hex_data[0] = pBuf[0];
	intData.Hex_data[1] = pBuf[1];
	intData.Hex_data[2] = pBuf[2];
	intData.Hex_data[3] = pBuf[3];

	return intData.int_data;
}
//float转字符
void AfxFloatToData(unsigned char *pBuf, float Data)
{
	Float_Data floatData;
	floatData.float_data = Data;
	pBuf[0] = floatData.Hex_data[0];
	pBuf[1] = floatData.Hex_data[1];
	pBuf[2] = floatData.Hex_data[2];
	pBuf[3] = floatData.Hex_data[3];
}

//字符转float
float AfxDataToFloat(unsigned char *pBuf)
{
	Float_Data floatData;
	floatData.Hex_data[0] = pBuf[0];
	floatData.Hex_data[1] = pBuf[1];
	floatData.Hex_data[2] = pBuf[2];
	floatData.Hex_data[3] = pBuf[3];

	return floatData.float_data;
}

/*字符串转int函数*/
unsigned int  str2int(char *str)
{
	if(strncmp(str,"0x",2) == 0)
      	return (unsigned int)strtol(str, NULL, 16);
    else
      	return (unsigned int)atoi(str);
}

/*校验和计算*/
Uint8 MakeSum(Uint8 *buf,Uint16 len)
{
   Uint16 i=0;
   Uint16 sum=0;
	for(i=0;i<len-1;i++)
	{
	   sum+=buf[i];
	}

	sum &= 0x00ff;
	return (Uint8)sum;
}


int xassert_fatal(const char *msg, int result){
    if (result < 0){
			perror(msg);
    }
    return result;
}

/*从文件读取cJSON结构*/
cJSON* GetJsonObjectFormfile(char* fileName)
{
    long len;
    char* pContent;
    int tmp;
    cJSON* json;
    FILE* fp = fopen(fileName, "r+");
    CHNLGPE_1Print("GetJsonObjectFormfile: Reading file : %s.\n",(Uint32)fileName);
    json = cJSON_CreateObject();
    if(!fp)
    {
        return NULL;
    }
    fseek(fp,0,SEEK_END);
    len=ftell(fp);
    if(0 == len)
    {

    		printf("no length\n");
        return NULL;
    }
    fseek(fp,0,SEEK_SET);
    pContent = (char*) malloc (sizeof(char)*len);
    tmp = fread(pContent,1,len,fp);
    fclose(fp);
    json=cJSON_Parse(pContent);
    if (!json)
    {
        return NULL;
    }
    free(pContent);
    return json;
}
