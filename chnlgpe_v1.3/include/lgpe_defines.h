/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_defines.h
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_defines.h
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  Chase  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _LGPE_DEFINES_H_
#define _LGPE_DEFINES_H_

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
#define SOCKET			1
#define BUFSIZE     256				//socket read长度

#define ADDR_GUARD_ARM 0x01									//保护板ARM
#define ADDR_GUARD_DSP 0x31									//保护板DSP
#define CHAR_ADDR_CTRL 0x11									//控制板
#define HEX_CMD_SET_PROJECT_PARAM	0x21	//设定工程参数
#define HEX_CMD_SET_GUARD_PARAM		0x22	//设定保护参数
#define HEX_CMD_SET_SAMPLE_PARAM		0x25	//设定采样通道参数
#define HEX_CMD_SET_CONTROL_PARAM   0x26    //设定控制策略参数
#define HEX_CMD_WORK_PATTERN		0x11	//工作模式切换
#define HEX_CMD_START_STOP			0x12	//系统启停命令
#define HEX_CMD_PULSE_ENABLE		0x13	//脉冲启停命令
#define HEX_CMD_RESET				0x15	//故障复位命令
#define HEX_CMD_BREAKER_OPERATION   0x14	//断路器动作命令
#define HEX_CMD_GET_RUNNING_DATA		0x31	//查询运行数据
#define HEX_CMD_GET_POWER_UNIT_DATA 0x32	//查询功率单元数据
#define HEX_CMD_GET_THD_DATA				0x33	//查询谐波数据
#define HEX_CMD_GET_PROJECT_PARAM   0x99  //查询工程参数（待定）
#define HEX_CMD_GET_GUARD_PARAM     0x98  //查询保护参数（待定）
#define ID_DEVICE_MODEL_SVG 0x11

#define HEX_RUNNING				(0x5A)	//运行模式
#define HEX_DEBUG_LOW			(0x50)	//低压调试模式
#define HEX_DEBUG_HIGH			(0x51)	//高压调试模式
#define HEX_START					(0x5A)   //启动
#define HEX_STOP					(0x50)   //停止
#define HEX_RESET					(0x5F) //复位
#define HEX_EM_STOP				(0x99)   //禁止停止
#define HEX_ACTION_ON			(0x5A)   //启动
#define HEX_ACTION_OFF			(0x50)   //停止

/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     		*
 *----------------------------------------------------------*/
typedef union _Int_Data
{
	int  int_data;
	unsigned char Hex_data[4];
}Int_Data;
typedef union _Float_Data
{
	float float_data;
	unsigned char Hex_data[4];
}Float_Data;
typedef union _Int16_Data
{
	Uint16 int16_data;
	unsigned char Hex_data[2];
}Int16_Data;

typedef struct _ErrCode
{
	unsigned char nErr;
	unsigned char nErr_para1;			//参数1
	float   		  fErr_para2;			//参数2
	unsigned char nErr_GuardLevel;	//保护等级
}ErrCode;
typedef struct _SystemRunningData
{
	unsigned char cSendId;	//发送方地址
	unsigned char cRevId;		//接收方地址
	unsigned char cAFN;			//命令代码

	float		  fUinAB;				//输入电压AB相有效值
	float		  fPhaseAngleUinAB;	//输入电压AB相相角
	float		  fUinBC;				//输入电压BC相有效值
	float		  fPhaseAngleUinBC;	//输入电压BC相相角
	float		  fUinCA;				//输入电压CA相有效值
	float		  fPhaseAngleUinCA;	//输入电压CA相相角

	float		  fUoutAB;			//输出电压AB相有效值
	float		  fPhaseAngleUoutAB;	//输出电压AB相相角
	float		  fUoutBC;			//输出电压BC相有效值
	float		  fPhaseAngleUoutBC;	//输出电压BC相相角
	float		  fUoutCA;			//输出电压CA相有效值
	float		  fPhaseAngleUoutCA;	//输出电压CA相相角

	float		  fIinAB;				//输入电流AB相有效值
	float		  fPhaseAngleIinAB;		//输入电流AB相相角
	float		  fIinBC;				//输入电流BC相有效值
	float		  fPhaseAngleIinBC;		//输入电流BC相相角
	float		  fIinCA;				//输入电流CA相有效值
	float		  fPhaseAngleIinCA;		//输入电流CA相相角

	float		  fIoutAB;				//输出电流AB相有效值
	float		  fPhaseAngleIoutAB;		//输出电流AB相相角
	float		  fIoutBC;				//输出电流BC相有效值
	float		  fPhaseAngleIoutBC;		//输出电流BC相相角
	float		  fIoutCA;				//输出电流CA相有效值
	float		  fPhaseAngleIoutCA;		//输出电流CA相相角

	float			fUdcP;						//直流正向电容电压
	float			fUdcN;  					//直流负向电容电压

	float			fPinAB;					//输入A相有功功率
	float			fPinBC;					//输入B相有功功率
	float			fPinCA;					//输入C相有功功率

	float			fQinAB;					//输入A相无功功率
	float			fQinBC;					//输入B相无功功率
	float			fQinCA;					//输入C相无功功率

	float			fPoutAB;					//输出A相有功功率
	float			fPoutBC;					//输出B相有功功率
	float			fPoutCA;					//输出C相有功功率

	float			fQoutAB;					//输出A相无功功率
	float			fQoutBC;					//输出B相无功功率
	float			fQoutCA;					//输出C相无功功率

	float			fPFinAB;					//输入A相功率因数
	float			fPFinBC;					//输入B相功率因数
	float			fPFinCA;					//输入C相功率因数

	float			fPFoutAB;				//输出A相功率因数
	float			fPFoutBC;			//输出B相功率因数
	float			fPFoutCA;				//输出C相功率因数

	float     fTHDinA;				//输入电压A相谐波畸变率
	float			fTHDinB;
	float			fTHDinC;
	float			fTHDoutA;				//输出电压A相谐波畸变率
	float			fTHDoutB;
	float			fTHDoutC;

	unsigned char cDI1;					//开入量1
	unsigned char cDI2;					//开入量2
	unsigned char cDI3;					//开入量3
	unsigned char cDI4;					//开入量4
	unsigned char cDI5;					//开入量5

	unsigned char cDO1;					//开出量1
	unsigned char cDO2;					//开出量2
	unsigned char cDO3;					//开出量3

	float			fTemperature;			//功率单元温度
	unsigned char	cComStatus;	//功率单元通讯状态
	unsigned char cWorkStatus;	//功率单元工作状态
	unsigned char cIGBTStatus1; //IGBT状态1
	unsigned char cIGBTStatus2; //IGBT状态2
	unsigned char cSystemRunStatus;	//系统运行状态
	unsigned char cWorkingMode;		//工作模式

	unsigned char cParamReady1;			//参数有效标志1
	unsigned char cParamReady2;			//参数有效标志2

	ErrCode  nErr[10];

}SystemRunningData;//系统运行数据
typedef struct _CmdOnOff
{
	unsigned char cSendId;		//发送方地址
	unsigned char cRevId;		//接收方地址
	unsigned char cAFN;			//命令代码
	unsigned char cParam;		//参数
}CmdOnOff;//系统启停 脉冲启停 故障复归 工作模式切换

typedef struct _BreakerOperate
{
	unsigned char cSendId;		//发送方地址
	unsigned char cRevId;		//接收方地址
	unsigned char cAFN;			//命令代码
	unsigned char cOrder;		//控制对象
	unsigned char cSwitch;		//开合状况0x55 断开（停止）， 0xaa 投入（启动）
}BreakerOperate;//断路器动作命令
typedef struct _OperateCommand
{
    unsigned char cSendId;	 //发送方地址
    unsigned char cRevId;	 //接收方地址
    unsigned char cAFN;	 //命令代码
    unsigned char cOrder;	 //控制对象
    unsigned char cParam;	 //参数
}OperateCommand;//系统启停 脉冲启停 故障复归 工作模式切换 断路器动作命令
typedef struct _SetParamRecv
{
		unsigned char cSendId;		//发送方地址
		unsigned char cRevId;    //接收方地址
		unsigned char cAFN;				//命令代码
}SetParamRecv;//参数设置返回

typedef struct _EngineeringParam
{
	unsigned char cSendId;		//发送方地址
	unsigned char cRevId;		//接收方地址
	unsigned char cAFN;			//命令代码

	float		  fPowerRated;			//额定功率
	float			fUinRated;				//额定电压
	float			fIinRated;				//输入额定电流
	float			fIoutRated;				//输出额定电流
	float			fUdcRated;				//直流电容额定电压
	float		  fPTin;				//输入电压互感器变比
	float		  fCTin;				//输入电流互感器变比
	float		  fPTout;				//输出电压互感器变比
	float		  fCTout;				//输出电流互感器变比
	float		  fPTcap;				//电容电压互感器变比
	float			fLin;					//输入电抗器电感
	float			fChargStopVoltage;	//功率单元电容充电终止电压
	float			fDischargeStopVoltage;	//功率单元电容放电终止电压
}EngineeringParam;//工程参数
typedef struct _ProtectParam
{
	unsigned char cSendId;		//发送方地址
	unsigned char cRevId;		//接收方地址
	unsigned char cAFN;			//命令代码

	unsigned char	cUinProtectionlvl;   //输入电压保护等级
	unsigned char cIinProtectionlvl;	 //输入过流保护等级
	unsigned char	cUoutProtectionlvl;   //输入电压保护等级
	unsigned char cIoutProtectionlvl;	 //输入过流保护等级
	unsigned char	cUDCProtectionlvl;   //直流电容保护
	unsigned char cFreqProtectionlvl;	 //频率保护等级
	unsigned char cPowerUnitProtectionlvl; //功率单元保护等级


	float 	 fOverVoltageInputK;			//输入电压过压倍数
	float		 fUnderVoltageInputK;    //输入电压欠压倍数
	float		 fUnbalanceInputK;				//输入电压三相不平衡保护比例

	float	   fOverCurrentInputK0;		//输入电流过流速断电流倍数
	float 	 fOverCurrentInputK1;		//输入电流过流一段电流倍数
	float		 fOverCurrentInputK2;		//输入电流过流二段电流倍数

	float		 fOverCurrentInputDelay1;//输入电流一段过流时间
	float	   fOverCurrentInputDelay2;//输入电流二段过流时间

	float		 fOverVoltageOutputK;		//输出电压过压倍数
	float		 fUnderVoltageOutputK;	//输出电压欠压倍数
	float 	 fUnbalanceOutputK;			//输出电压三相不平衡保护比例

	float		 fOverCurrentOutputK0;		//输出电流过流速断电流倍数
	float		 fOverCurrentOutputK1;		//输出电流过流一段电流倍数
	float		 fOverCurrentOutputK2;		//输出电流过流二段电流倍数
	float		 fOverCurrentOutputDelay1;//输出电流一段过流时间
	float		 fOverCurrentOutputDelay2;//输出电流二段过流时间

	float    fDCOverVoltageK0;				//直流电容过压速断电压
	float		 fDCOverVoltageK1;				//直流电容过压预警电压
	float    fDCUnderVoltageK;				//直流电容电压下限
	float		 fDCVoltageUnbalanceK;		//直流电容不平衡系数

	float	   fUpperFreqLimit;				//输入电压频率上限
	float		 fLowerFreqLimit;       //输入电压频率下限

	float		 fTemperatureLimit0;		//功率单元速断温度
	float		 fTemperatureLimit1;		//功率单元温度超限预警温度
}ProtectParam;//保护参数

typedef struct _ControlPolicyParam
{
	unsigned char cSendId;		//发送方地址
	unsigned char cRevId;		//接收方地址
	unsigned char cAFN;			//命令代码

	float fVbusRef;				//母线电压目标设定值
	float fVbusBase;			//母线电压基准值
	float fVcapRef;				//电容电压目标设定值
	float fVcapBase;			//电容电压基准值
	float fVbusKp;				//母线电压闭环比例系数
	float fVbusKi;				//母线电压闭环积分系数
	float fVcapKp;				//电容电压闭环比例系数
	float fVcapKi;				//电容电压闭环积分系数
	float IdKp;					//电流闭环Id比例系数
	float IdKi;					//电流闭环Id积分系数
	float IqKp;					//电流闭环Iq比例系数
	float IqKi;					//电流闭环Iq积分系数
}ControlPolicyParam;//控制策略参数

typedef struct _HarmonicData
{
	unsigned char cSendId;
	unsigned char cRevId;
	unsigned char cAFN;

	float fUinHarmonicA[31];
	float fUinHarmonicB[31];
	float fUinHarmonicC[31];
	float fUoutHarmonicA[31];
	float fUoutHarmonicB[31];
	float fUoutHarmonicC[31];
}HarmonicData;
typedef struct _SysRunHarmData
{
  SystemRunningData RunData;
  HarmonicData      HarmData;
}SysRunHarmData;
/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     		*
 *----------------------------------------------------------*/

/********************************************************************************
发送结构体
********************************************************************************/
volatile OperateCommand g_sOperateCommand;							//命令发送结构体全局变量
volatile EngineeringParam g_sSendEngineeringParam;	//工程参数发送全局变量
volatile ProtectParam g_sSendProtectParam;					//保护参数发送全局变量
volatile ControlPolicyParam g_sSendControlPolicyParam; //控制策略参数发送全局变量

/********************************************************************************
接收结构体
********************************************************************************/
volatile SystemRunningData g_SystemRunningData;				//运行数据结构体全局变量
volatile HarmonicData g_sHarmonicData;						//谐波数据结构体全局变量
volatile SysRunHarmData g_sRunHarmData ;					//谐波和运行数据全局变量

volatile OperateCommand g_sRecvSystemOnOff;				//系统启停返回全局变量
volatile OperateCommand g_sRecvPulseEnable;				//脉冲启停返回全局变量
volatile OperateCommand g_sRecvWorkPattern;				//工作模式切换返回全局变量
volatile OperateCommand g_sRecvFailReset;					//故障复归返回全局变量
volatile OperateCommand g_sRecvBreakOperate;	//断路器动作返回全局变量
volatile SetParamRecv g_sSetParamRecv;				//保护、工程参数设置返回全局变量
volatile EngineeringParam g_sRecvEngineeringParam;	//工程参数返回全局变量
volatile ProtectParam g_sRecvProtectParam;					//保护参数返回全局变量
volatile ControlPolicyParam g_sRecvControlPolicyParam; //控制策略参数返回全局变量

/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    	*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   	*
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _LGPE_DEFINES_H_ */
