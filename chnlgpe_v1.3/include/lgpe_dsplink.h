/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_dsplink.h
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_dsplink.h
Create Date  : 2014年3月14日
Description  : 

Modification History
REV1.0.0  Chase  2014年3月14日  File Create

******************************************************************************/

/*----------------------------------------------------------*
 * 避免多次包含起始说明 Multi-Include-Prevent Start Section  *
 *----------------------------------------------------------*/

#ifndef _LGPE_DSPLINK_H_
#define _LGPE_DSPLINK_H_

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*----------------------------------------------------------*
 * 调试开关             Debug Switch Section            *
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 包含文件             Include File Section            *
 *----------------------------------------------------------*/

#include "dsplink.h"

/*----------------------------------------------------------*
 * 全局宏定义           Global Macro Define Section         		*
 *----------------------------------------------------------*/
/** ============================================================================
 *  @const  CHNL_ID_OUTPUT
 *
 *  @desc   ID of channel used to send data to DSP.
 *  ============================================================================
 */
#define CHNL_ID_OUTPUT     0


/** ============================================================================
 *  @const  CHNL_ID_INPUT
 *
 *  @desc   ID of channel used to receive data from DSP.
 *  ============================================================================
 */
#define CHNL_ID_INPUT      1

/** ============================================================================
 *  @const  CHNL_PACKET_LEN
 *
 *  @desc   DSP/Link数据包长度.
 *  ============================================================================
 */
#define CHNL_PACKET_LEN		(1024)

/*----------------------------------------------------------*
 * 全局结构定义         Global Structure Define Section     		*
 *----------------------------------------------------------*/
/** ============================================================================
 *  @const  LGPEPara
 *
 *  @desc   para of CreateDSPLink.
 *  ============================================================================
 */
typedef struct _LGPEPara{
	IN Char8 * dspExecutable;
	IN Char8 * strBufferSize;
	IN Char8 * strNumIterations;
	IN Char8 * processorId;
}LGPEPara;
typedef struct lgpe_dsplink_resources
{
	pthread_mutex_t dsplink_txbuffer_mutex;
	pthread_mutex_t dsplink_rxbuffer_mutex;
	sem_t dsplink_tx_sem;
	sem_t dsplink_rx_sem;
	int tx_length;
	int rx_length;
	unsigned char tx_buffer[CHNL_PACKET_LEN];
	unsigned char rx_buffer[CHNL_PACKET_LEN];
}LGPE_dsplink_res;
/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     		*
 *----------------------------------------------------------*/

extern LGPE_dsplink_res *g_dsplink_shm;

/*----------------------------------------------------------*
 * 全局函数原型声明     Global Prototype Declare Section    	*
 *----------------------------------------------------------*/

/** ============================================================================
 *  @func   lgpe_dsplink_shm_create
 *
 *  @desc   创建LGPE_dsplink共享内存区域；
 *
 *  @arg    name
 *              共享内存区域名称；
 *  @ret    LGPE_dsplink_res *
 *              返回LGPE_dsplink_res指针；
 *          NULL
 *              共享内存区域初始化失败；
 *  ============================================================================
 */
LGPE_dsplink_res *lgpe_dsplink_shm_create(char *name);

/** ============================================================================
 *  @func   lgpe_dsplink_shm_delete
 *
 *  @desc   回收LGPE_dsplink共享内存资源；
 *
 *  @arg    dsplink_res
 *              共享内存区域指针；
 *  @ret    void
 *  ============================================================================
 */
void lgpe_dsplink_shm_delete(LGPE_dsplink_res *dsplink_res_p);


/*外部函数*/
extern void sigroutine(int dunno);

/** ============================================================================
 *  @func   CHNLGPE_Create
 *
 *  @desc   This function allocates and initializes resources used by
 *          this application.
 *
 *  @arg    dspExecutable
 *              DSP executable name.
 *  @arg    bufferSize
 *              String representation of buffer size to be used
 *              for data transfer.
 *  @arg    strNumIterations
 *              Number of iterations a data buffer is transferred between
 *              GPP and DSP in string format.
 *
 *  @arg    processorId
 *             Id of the DSP Processor.
 *
 *  @ret    DSP_SOK
 *              Operation Successfully completed.
 *          DSP_EFAIL
 *              Resource allocation failed.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    CHNLGPE_Delete
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
CHNLGPE_Create (IN Char8 * dspExecutable,
			 IN Char8 * strBufferSize,
			 IN Char8 * strNumIterations,
			 IN Uint8   processorId) ;


/** ============================================================================
 *  @func   CHNLGPE_Execute
 *
 *  @desc   This function implements the execute phase for this application.
 *
 *  @arg    numIterations
 *              Number of iterations a data buffer is transferred between
 *              GPP and DSP in string format.
 *
 *  @ret    DSP_SOK
 *              Operation Successfully completed.
 *          DSP_EFAIL
 *              Loop execution failed.
 *
 *  @arg    processorId
 *             Id of the DSP Processor.
 *
 *  @ret    DSP_SOK
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    CHNLGPE_Delete , CHNLGPE_Create
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
CHNLGPE_Execute (IN Uint32 numIterations, IN Uint8 processorId) ;


/** ============================================================================
 *  @func   CHNLGPE_Delete
 *
 *  @desc   This function releases resources allocated earlier by call to
 *          CHNLGPE_Create ().
 *          During cleanup, the allocated resources are being freed
 *          unconditionally. Actual applications may require stricter check
 *          against return values for robustness.
 *
 *  @arg    processorId
 *             Id of the DSP Processor.
 *
 *  @ret    DSP_SOK
 *
 *  @ret    DSP_SOK
 *              Operation Successfully completed.
 *          DSP_EFAIL
 *              Resource deallocation failed.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    CHNLGPE_Create
 *  ============================================================================
 */
NORMAL_API
Void
CHNLGPE_Delete (Uint8 processorId) ;


/** ============================================================================
 *  @func   CHNLGPE_Main
 *
 *  @desc   The OS independent driver function for the loop sample application.
 *
 *  @arg    dspExecutable
 *              Name of the DSP executable file.
 *  @arg    strBufferSize
 *              Buffer size to be used for data-transfer in string format.
 *  @arg    strNumIterations
 *              Number of iterations a data buffer is transferred between
 *              GPP and DSP in string format.
 *
 *  @arg    processorId
 *             Id of the DSP Processor in string format.
 *
 *  @ret    DSP_SOK
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    CHNLGPE_Create, CHNLGPE_Execute, CHNLGPE_Delete
 *  ============================================================================
 */
NORMAL_API
Void *
CHNLGPE_Main (void *args) ;

#if defined (DA8XXGEM)
/** ============================================================================
 *  @func   CHNLGPE_Main_DA8XX
 *
 *  @desc   The OS independent driver function for the loop sample application.
 *
 *  @arg    dspExecutable
 *              Name of the DSP executable file.
 *  @arg    strBufferSize
 *              Buffer size to be used for data-transfer in string format.
 *  @arg    strNumIterations
 *              Number of iterations a data buffer is transferred between
 *              GPP and DSP in string format.
 *  @arg    processorId
 *             Id of the DSP Processor in string format.
 *  @arg    strDspAddr
 *             c_int00 address
 *  @arg    strShmAddr
 *             DSPLINK_shmBaseAddress address
 *  @arg    strArgsAddr
 *             .args section address
 *
 *  @ret    DSP_SOK
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    CHNLGPE_Create, CHNLGPE_Execute, CHNLGPE_Delete
 *  ============================================================================
 */
NORMAL_API
Void
CHNLGPE_Main_DA8XX (IN Char8 * dspExecutable,
				 IN Char8 * strBuffersize,
				 IN Char8 * strNumIterations,
				 IN Char8 * processorId,
				 IN Char8 * strDspAddr,
				 IN Char8 * strShmAddr,
				 IN Char8 * strArgsAddr) ;
#endif

/** ============================================================================
 *  @func   chnlSendCmdFrame
 *
 *  @desc   发送命令给DSP,DSP/Link唯一可调用的发送函数；
 *
 *  @arg    Send_Str
 *              发送数据指针；
 *          length
 *          	发送数据包长度；
 *  @ret    void
 *  ============================================================================
 */
void chnlSendCmdFrame(unsigned char *Send_Str,int length);

/*----------------------------------------------------------*
 * 避免多次包含的结束   Multi-Include-Prevent End Section   	*
 *----------------------------------------------------------*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _LGPE_DSPLINK_H_ */
