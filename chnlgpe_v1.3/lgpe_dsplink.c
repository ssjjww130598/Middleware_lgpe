/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.2
文 件 名 : lgpe_dsplink.c
创建日期 : 2014年3月14日
文件说明 : 

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.2
File Name    : lgpe_dsplink.c
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

/*----------------------------------------------------------*
 * 局部宏定义         Local Macro Define Section            		*
 *----------------------------------------------------------*/
/*  ============================================================================
 *  @name   NUM_ARGS
 *
 *  @desc   Number of arguments specified to the DSP application.
 *  ============================================================================
 */
#define NUM_ARGS 2

/*  ============================================================================
 *  @name   XFER_CHAR
 *
 *  @desc   The value used to initialize the output buffer and used for
 *          validation against the input buffer recieved.
 *  ============================================================================
 */
#define XFER_CHAR   (Char8) 0xE7

/** ============================================================================
 *  @name   NUMBUFFERPOOLS
 *
 *  @desc   Number of buffer pools in this application.
 *  ============================================================================
 */
#define NUMBUFFERPOOLS 1

/** ============================================================================
 *  @name   NUMBUFS
 *
 *  @desc   Number of buffers in pool.
 *  ============================================================================
 */
#define NUMBUFS        16

/** ============================================================================
 *  @name   POOL_ID
 *
 *  @desc   Pool id for this application.
 *  ============================================================================
 */
#define POOL_ID        0
/*----------------------------------------------------------*
 * 局部结构定义       Local Structure Define Section        	*
 *----------------------------------------------------------*/
/*  ============================================================================
 *  @name   CHNLGPE_BufferSize
 *
 *  @desc   Size of buffer to be used for data transfer.
 *  ============================================================================
 */
STATIC Uint32  CHNLGPE_BufferSize = 1024;

/** ============================================================================
 *  @name   CHNLGPE_Buffers
 *
 *  @desc   Array of buffers used by the chnlgpe application.
 *          Length of array in this application is 1.
 *  ============================================================================
 */
STATIC Char8 * CHNLGPE_Buffers [2] ;

/** ============================================================================
 *  @name   CHNLGPE_IOReq
 *
 *  @desc   It gives information for adding or reclaiming a request.
 *  ============================================================================
 */
STATIC ChannelIOInfo CHNLGPE_IOReq  ;
STATIC ChannelIOInfo CHNLGPE_CMD_Send  ;

#if defined (DA8XXGEM)
/** ============================================================================
 *  @name   dspAddr
 *
 *  @desc   Address of c_int00 in the DSP executable.
 *  ============================================================================
 */
Uint32 CHNLGPE_dspAddr ;

/** ============================================================================
 *  @name   shmAddr
 *
 *  @desc   Address of symbol DSPLINK_shmBaseAddres in the DSP executable.
 *  ============================================================================
 */
Uint32 CHNLGPE_shmAddr ;

/** ============================================================================
 *  @name   argsAddr
 *
 *  @desc   Address of .args section in the DSP executable.
 *  ============================================================================
 */
Uint32 CHNLGPE_argsAddr ;

/** ============================================================================
 *  @name   LINKCFG_config
 *
 *  @desc   Extern declaration to the default DSP/BIOS LINK configuration
 *          structure.
 *  ============================================================================
 */
extern  LINKCFG_Object LINKCFG_config ;

#endif

LGPEconnListList* g_server_clients_list;
//XConnList*  g_org_server_clients;

//pthread_t tid1;
//pthread_t tid2;
//pthread_t tid3;
extern pthread_mutex_t mutex;
int retry_count = 0;
//sem_t sem;
extern volatile int bExit;
extern volatile int g_dsplinkExit;
volatile int bdsplinkexit = 0;
pthread_t tid_dsplink_senddaemon;
int g_dsplink_shm_fd;
char g_dsplink_shm_name[128];
LGPE_dsplink_res *g_dsplink_shm = NULL;
extern cJSON *g_jsonOut;

/*----------------------------------------------------------*
 * 局部函数原型声明   Local Prototype Declare Section       	*
 *----------------------------------------------------------*/

/** ============================================================================
 *  @func   thrd_Dsplink_SendDaemon
 *
 *  @desc   This thread sends data to dsp when data's ready.
 *
 *  @modif  None
 *  ============================================================================
 */
void *thrd_Dsplink_SendDaemon();

/** ============================================================================
 *  @func   thrd_Dsplink_Recv
 *
 *  @desc   This thread receives data from dsp.
 *
 *  @modif  None
 *  ============================================================================
 */
void thrd_Dsplink_Recv();
/*----------------------------------------------------------*
 * 静态变量定义       Static Variable Define Section        	*
 *----------------------------------------------------------*/

/*----------------------------------------------------------*
 * 函数定义           Function Define Section               		*
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
LGPE_dsplink_res *lgpe_dsplink_shm_create(char *name)
{
	int ret;
	/*新建共享内存块*/
	strcpy(g_dsplink_shm_name, name);
	g_dsplink_shm_fd = shm_open(g_dsplink_shm_name, O_RDWR|O_CREAT,
		S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP);
	if (g_dsplink_shm_fd < 0)
	{
		perror("lgpe_dsplink_shm_create: dsplink shm_open error! ");
		return NULL;
	}
	/*调整文件共享内存空间*/
	ret = ftruncate(g_dsplink_shm_fd, sizeof(LGPE_dsplink_res));
	if (ret < 0)
	{
		perror("lgpe_dsplink_shm_create: dsplink ftruncate error! ");
		return NULL;
	}
	/*映射共享文件到内存地址*/
	LGPE_dsplink_res *dsplink_addr = mmap(NULL, sizeof(LGPE_dsplink_res),
		PROT_READ|PROT_WRITE, MAP_SHARED, g_dsplink_shm_fd, SEEK_SET);
	if(!dsplink_addr)
	{
		perror("lgpe_dsplink_shm_create: dsplink mmap error! ");
		return NULL;
	}

	if(dsplink_addr)
	{
//		dsplink_addr->dsplinkExit = 0;
		/*初始化mutex*/
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);		/*设置为进程间共享*/
		pthread_mutex_init(&dsplink_addr->dsplink_txbuffer_mutex, &attr);
		pthread_mutex_init(&dsplink_addr->dsplink_rxbuffer_mutex, &attr);

		/*初始化semaphore*/
		/*int sem_init(sem_t *sem, int pshared, unsigned int value);
				pshared = 0 信号量进程内共享； pshared != 0 信号量进程之间共享；
			*/
		sem_init(&dsplink_addr->dsplink_tx_sem,1, 0);
		sem_init(&dsplink_addr->dsplink_rx_sem,1, 0);
	}
	return dsplink_addr;
}
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
void lgpe_dsplink_shm_delete(LGPE_dsplink_res *dsplink_res_p)
{
	int ret;

	/*释放互斥锁和信号灯资源*/
	pthread_mutex_destroy(&g_dsplink_shm->dsplink_txbuffer_mutex);
	pthread_mutex_destroy(&g_dsplink_shm->dsplink_rxbuffer_mutex);
	sem_destroy(&g_dsplink_shm->dsplink_tx_sem);
	sem_destroy(&g_dsplink_shm->dsplink_rx_sem);

	ret = munmap((void *)dsplink_res_p, sizeof(LGPE_dsplink_res));
	if(ret)
		perror("lgpe_dsplink_shm_delete: dsplink munmap error! ");
	ret = shm_unlink(g_dsplink_shm_name);
	if(ret)
		perror("lgpe_dsplink_shm_delete: dsplink shm_unlink error! ");
}

/** ============================================================================
 *  @func   CHNLGPE_Create
 *
 *  @desc   This function allocates and initializes resources used by
 *          this application.
 *
 *  @modif  CHNLGPE_Buffers
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
CHNLGPE_Create (IN Char8 * dspExecutable,
             IN Char8 * strBufferSize,
             IN Char8 * strNumIterations,
             IN Uint8   processorId)
{
    DSP_STATUS          status                    = DSP_SOK   ;
//    Char8 *             temp                      = NULL      ;
    Uint32              numArgs                   = 0         ;
    Uint32              numBufs [NUMBUFFERPOOLS]  = {NUMBUFS} ;
#if defined (DA8XXGEM)
    NOLOADER_ImageInfo  imageInfo ;
#endif

    ChannelAttrs  chnlAttrInput            ;
    ChannelAttrs  chnlAttrOutput           ;
//    Uint16        i                        ;
    Char8 *       args [NUM_ARGS]          ;
    Uint32        size [NUMBUFFERPOOLS]    ;
#if defined (ZCPY_LINK)
    SMAPOOL_Attrs poolAttrs                ;
#endif /* if defined (ZCPY_LINK) */


CHNLGPE_0Print ("CHNLGPE_Main: Entered CHNLGPE_Create ()\n") ;

    /*
     *  Create and initialize the proc object.
     */
    status = PROC_setup (NULL) ;

    /*
     *  Attach the Dsp with which the transfers have to be done.
     */
    if (DSP_SUCCEEDED (status)) {
        status = PROC_attach (processorId, NULL) ;
        if (DSP_FAILED (status)) {
            CHNLGPE_1Print ("CHNLGPE_Main: PROC_attach failed . Status = [0x%x]\n", status) ;
        }
    }
    else {
        CHNLGPE_1Print ("CHNLGPE_Main: PROC_setup failed. Status =  [0x%x]\n", status) ;
    }

    /*
     *  Open the pool.
     */
    if (DSP_SUCCEEDED (status)) {
        size [0] = CHNLGPE_BufferSize ;
        poolAttrs.bufSizes      = (Uint32 *) &size ;
        poolAttrs.numBuffers    = (Uint32 *) &numBufs ;
        poolAttrs.numBufPools   = NUMBUFFERPOOLS ;
#if defined (ZCPY_LINK)
        poolAttrs.exactMatchReq = TRUE ;
#endif /* if defined (ZCPY_LINK) */
        status = POOL_open (POOL_makePoolId(processorId, POOL_ID), &poolAttrs) ;
        if (DSP_FAILED (status)) {
            CHNLGPE_1Print ("CHNLGPE_Main: POOL_open () failed. Status = [0x%x]\n",
                            status) ;
        }
		printf("CHNLGPE_Main: POOL attrs: bufsizes = %xH; numBuffers = %d;\n",
						(unsigned int)(*poolAttrs.bufSizes),(int)(*poolAttrs.numBuffers));
    }
#if 0
	// Initialize msgq_debug
	DEBUG_Create(DEBUG_POOL_ID);
	CHNLGPE_0Print("CHNLGPE_Main: Finished initialise MSGQ_debug. press any key to continue.\n");
	getchar();
#endif
    /*
     *  Load the executable on the DSP.
     */
    if (DSP_SUCCEEDED (status)) {
        numArgs  = NUM_ARGS         ;
        args [0] = strBufferSize    ;
        args [1] = strNumIterations ;

#if defined (DA8XXGEM)
         if (LINKCFG_config.dspConfigs [processorId]->dspObject->doDspCtrl ==
                     DSP_BootMode_NoBoot) {
            CHNLGPE_0Print ("CHNLGPE_Main: Start Loop with noboot. \n");
            imageInfo.dspRunAddr  = CHNLGPE_dspAddr ;
            imageInfo.shmBaseAddr = CHNLGPE_shmAddr ;
            imageInfo.argsAddr    = CHNLGPE_argsAddr ;
            imageInfo.argsSize    = 50         ;
            status = PROC_load (processorId, (Char8 *) &imageInfo, numArgs, args) ;
        }
        else
#endif
        {
            status = PROC_load (processorId, dspExecutable, numArgs, args) ;
        }
       if (DSP_FAILED (status)) {
            CHNLGPE_1Print ("CHNLGPE_Main: PROC_load failed. Status = [0x%x]\n", status) ;
        }
    }

    /*
     *  Create a channel to DSP
     */
    if (DSP_SUCCEEDED (status)) {
        chnlAttrOutput.mode      = ChannelMode_Output     ;
        chnlAttrOutput.endianism = Endianism_Default      ;
        chnlAttrOutput.size      = ChannelDataSize_16bits ;

        status = CHNL_create (processorId, CHNL_ID_OUTPUT, &chnlAttrOutput) ;
        if (DSP_FAILED (status)) {
            CHNLGPE_1Print ("CHNLGPE_Main: CHNL_create failed (output). Status = [0x%x]\n",
                         status) ;
        }
    }

    /*
     *  Create a channel from DSP
     */
    if (DSP_SUCCEEDED (status)) {
        chnlAttrInput.mode      = ChannelMode_Input      ;
        chnlAttrInput.endianism = Endianism_Default      ;
        chnlAttrInput.size      = ChannelDataSize_16bits ;

        status = CHNL_create (processorId, CHNL_ID_INPUT, &chnlAttrInput) ;
        if (DSP_FAILED (status)) {
            CHNLGPE_1Print ("CHNLGPE_Main: CHNL_create failed (input). Status = [0x%x]\n",
                         status) ;
        }
    }

    /*
     *  Allocate buffer(s) for data transfer to DSP.
     */
    if (DSP_SUCCEEDED (status)) {
        status = CHNL_allocateBuffer (processorId,
                                      CHNL_ID_OUTPUT,
                                      CHNLGPE_Buffers,
                                      CHNLGPE_BufferSize ,
                                      2) ;
        if (DSP_FAILED (status)) {
            CHNLGPE_1Print ("CHNLGPE_Main: CHNL_allocateBuffer failed (output)."
                         " Status = [0x%x]\n",
                         status) ;
        }
    }

    CHNLGPE_0Print ("CHNLGPE_Main: Leaving CHNLGPE_Create ()\n") ;

    return status ;
}

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
void chnlSendCmdFrame(unsigned char *Send_Str,int length)
{
	if(length <= CHNL_PACKET_LEN)
	{
		memcpy(g_dsplink_shm->tx_buffer, Send_Str, (size_t)length);
		g_dsplink_shm->tx_length = length;
		sem_post(&g_dsplink_shm->dsplink_tx_sem);
		lgdebug(DBG_1,"chnlSendCmdFrame: dsplink_tx_sem posted.\n");
	}
}

/*压入发送队列*/
void CHNL_send_frame(unsigned char *Send_Str,int length)
{
	DSP_STATUS status = DSP_SOK ;
	Uint8 processorId = 0;
//	int i;
	memset(CHNLGPE_CMD_Send.buffer,0,CHNLGPE_CMD_Send.size);
	if(length > CHNL_PACKET_LEN)
		return;
	/*
	for(i=0;i<lenth;i++)
	{
  		CHNLGPE_CMD_Send.buffer[i] =  Send_Str[i];
	}*/
	memcpy(CHNLGPE_CMD_Send.buffer, Send_Str, length);
	/*
	 *	Send data to DSP.
	 *	Issue 'filled' buffer to the channel.
	 */
	status = CHNL_issue (processorId, CHNL_ID_OUTPUT, &CHNLGPE_CMD_Send) ;
	if (DSP_FAILED (status))
	{
		CHNLGPE_1Print ("CHNL_send_frame: CHNL_issue failed (output). Status = [0x%x]\n",
					  status) ;
	}
	/*
	 *  Reclaim 'empty' buffer from the channel
	 */
	if (DSP_SUCCEEDED (status))
	{
	    status = CHNL_reclaim (processorId,
	                           CHNL_ID_OUTPUT,
	                           WAIT_FOREVER,
	                           &CHNLGPE_CMD_Send) ;
	    if (DSP_FAILED (status)) {
	        CHNLGPE_1Print ("CHNL_send_frame: CHNL_reclaim failed (output). Status = [0x%x]\n",
	                     status) ;
	    }
	}


}
/** ============================================================================
 *  @func   thrd_Dsplink_SendDaemon
 *
 *  @desc   This thread sends data to dsp when data's ready.
 *
 *  @modif  None
 *  ============================================================================
 */
void *thrd_Dsplink_SendDaemon()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	for(;;)
	{
		if (g_dsplinkExit == 1)
		{
			return NULL;
		}
		sem_wait(&g_dsplink_shm->dsplink_tx_sem);
		lgdebug(DBG_1,"thrd_Dsplink_SendDaemon: CHNL start sending frame length: %d.\n",g_dsplink_shm->tx_length);
		CHNL_send_frame(g_dsplink_shm->tx_buffer, g_dsplink_shm->tx_length);
	}

}
Void packageTestRunningData(Uint8 * buf)
{
	Uint32 i,j;

	i=2;
	memset(buf,0,1024);

	buf[i++] = ADDR_GUARD_ARM;					/*接收地址*/
	buf[i++] = ADDR_GUARD_DSP;					/*发送地址*/
	buf[i++] = ID_DEVICE_MODEL_SVG;				/*设备型号标识*/
	buf[i++] = 0;								/*DeviceID设备编号初始化为0，由ARM填充*/
	buf[i++] = HEX_CMD_GET_RUNNING_DATA;			/*命令代码*/

	/*==========运行数据==========*/
	AfxFloatToData(&buf[i], 10003.2 +rand()/1560.0);			i+=4;	/*母线电压A相有效值*/
	AfxFloatToData(&buf[i], 0.523);							i+=4;	/*母线电压A相相角*/
	AfxFloatToData(&buf[i], 9999.6 +rand()/1560.0);			i+=4;	/*母线电压B相有效值*/
	AfxFloatToData(&buf[i], 1.57);							i+=4;	/*母线电压B相相角*/
	AfxFloatToData(&buf[i], 9998.2 +rand()/1560.0);			i+=4;	/*母线电压C相有效值*/
	AfxFloatToData(&buf[i], 2.62);							i+=4;	/*母线电压C相相角*/

	AfxFloatToData(&buf[i], 0);			i+=4;	/*母线高压侧电压AB相有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*母线高压侧电压AB相相角*/
	AfxFloatToData(&buf[i], 0);			i+=4;	/*母线高压侧电压BC相有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*母线高压侧电压BC相相角*/
	AfxFloatToData(&buf[i], 0);			i+=4;	/*母线高压侧电压CA相有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*母线高压侧电压CA相相角*/

	AfxFloatToData(&buf[i], 11.0);							i+=4;	/*母线电流A相有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*母线电流A相相角*/
	AfxFloatToData(&buf[i], 12.0);							i+=4;	/*母线电流B相有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*母线电流B相相角*/
	AfxFloatToData(&buf[i], 13.0);							i+=4;	/*母线电流C相有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*母线电流C相相角*/

	AfxFloatToData(&buf[i], 11.0);							i+=4;	/*SVG A相电流有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*SVG A相电流相角*/
	AfxFloatToData(&buf[i], 12.0);							i+=4;	/*SVG B相电流有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*SVG B相电流相角*/
	AfxFloatToData(&buf[i], 13.0);							i+=4;	/*SVG C相电流有效值*/
	AfxFloatToData(&buf[i], 0);	i+=4;	/*SVG C相电流相角*/

#if 0
	AfxFloatToData(&buf[i],0);				i+=4;	/*A相直流电压*/
	AfxFloatToData(&buf[i],0);				i+=4;	/*B相直流电压*/
	AfxFloatToData(&buf[i],0);				i+=4;	/*C相直流电压*/

	AfxFloatToData(&buf[i],0);				i+=4;	/*A相直流电流*/
	AfxFloatToData(&buf[i],0);				i+=4;	/*B相直流电流*/
	AfxFloatToData(&buf[i],0);				i+=4;	/*C相直流电流*/
#endif

	AfxFloatToData(&buf[i], 0);			i+=4;	/*负载A相有功功率*/
	AfxFloatToData(&buf[i], 0);			i+=4;	/*负载B相有功功率*/
	AfxFloatToData(&buf[i], 0);			i+=4;	/*负载C相有功功率*/
	AfxFloatToData(&buf[i], 18933);							i+=4;	/*负载总有功功率*/

	AfxFloatToData(&buf[i], 0);			i+=4;	/*负载A相无功功率*/
	AfxFloatToData(&buf[i], 0);			i+=4;	/*负载B相无功功率*/
	AfxFloatToData(&buf[i], 0);			i+=4;	/*负载C相无功功率*/
	AfxFloatToData(&buf[i], 0);			i+=4;	/*负载总无功功率*/

	AfxFloatToData(&buf[i], 0);		i+=4;	/*负载A相功率因数*/
	AfxFloatToData(&buf[i], 0);		i+=4;	/*负载B相功率因数*/
	AfxFloatToData(&buf[i], 0);		i+=4;	/*负载C相功率因数*/
	AfxFloatToData(&buf[i], 0.983);							i+=4;	/*负载总功率因数*/

#if 0
	/* 打包功率单元状态*/
	for(j=0;j<g_SysParam.ProjectParam.PowerUnitNum;j++)
	{
		buf[i++] = g_SampleData.PowerUnit[j].Temperature & 0x00FF;	/*功率单元温度*/
		buf[i++] = g_SampleData.PowerUnit[j].Temperature >> 8;		/*功率单元温度高8位*/
		buf[i++] = g_SampleData.PowerUnit[j].Voltage & 0x00FF;		/*功率单元电压低8位*/
		buf[i++] = g_SampleData.PowerUnit[j].Voltage >> 8;			/*功率单元电压高8位*/
		buf[i++] = g_SampleData.PowerUnit[j].Status.all & 0x00FF;	/*功率单元状态低8位*/
		buf[i++] = g_SampleData.PowerUnit[j].Status.all >> 8;		/*功率单元状态高8位*/
	}
#endif
	/*==========开关量状态==========*/
	/*开入量*/
	buf[i++] = 0;		/*开入量1-8*/
	buf[i++] = 0;			/*开入量9-16*/
	buf[i++] = 0;		/*开入量17-24*/
	buf[i++] = 0;			/*开入量25-32*/
	buf[i++] = 0;		/*开出量33-39*/

	/*开出量*/
	buf[i++] = 0;		/*开出量1-8*/
	buf[i++] = 0;			/*开出量9-16*/
	buf[i++] = 0;		/*开出量17-24*/

	/*==========装置状态==========*/
	buf[i++] = 0;		/*通讯用系统运行状态*/
	buf[i++] = 0;				/*工作模式*/
	buf[i++] = 0;		/*参数有效标志1*/
	buf[i++] = 0;								/*参数有效标志2*/

	/*==========错误代码==========*/
	for(j=0;j<10;j++)
	{
		buf[i++] = 0;		/*错误代码*/
		buf[i++] = 0;		/*参数1*/
		AfxFloatToData(&buf[i], 0.0);		i+=4;		/*参数2*/
		buf[i++] = 0;	/*保护等级*/
	}
	buf[i++] = 2014 & 0xFF;
	buf[i++] = (2014 >> 8) & 0xFF;
	buf[i++] = 4 & 0xFF;
	buf[i++] = 1 & 0xFF;
	buf[i++] = 16 & 0xFF;
	buf[i++] = 6 & 0xFF;
	buf[i++] = 45 & 0xFF;

	buf[0] = (i+1) & 0xFF;
	buf[1] = (i+1) >> 8;
	buf[i] = MakeSum(buf,i);
	lgdebug(DBG_1,"packageTestRunningData: package with length: %d .\n", i+1);
}
/** ============================================================================
 *  @func   thrd_Dsplink_Recv
 *
 *  @desc   This thread receives data from dsp.
 *
 *  @modif  None
 *  ============================================================================
 */
Void thrd_Dsplink_Recv()
{
	unsigned char calsChar = 0;
	Uint16 nLength;
	Uint8 processorId = 0;
	DSP_STATUS status = DSP_SOK;
//	unsigned char *strOut;
	/*信号拦截 因为dsplink会拦截信号SIGINT所以在启动后将SIGINT抢回来*/
	signal(SIGINT, sigroutine);
	signal(SIGTERM, sigroutine);
	for (;;)
	{
		/*判断是否满足终止条件*/
		if (g_dsplinkExit == 1)
		{
			if (pthread_cancel(tid_dsplink_senddaemon) == 0)  // 关闭发送线程
			{
				CHNLGPE_0Print("thrd_Dsplink_Recv: thrd_Dsplink_SendDaemon terminated.\n");
			}
			pthread_join(tid_dsplink_senddaemon, NULL);
			/*回收共享内存资源*/
			lgpe_dsplink_shm_delete(g_dsplink_shm);
			return;
		}

		status = CHNL_issue(processorId, CHNL_ID_INPUT, &CHNLGPE_IOReq);
		if (DSP_FAILED(status))
		{
			CHNLGPE_1Print("thrd_Dsplink_Recv: CHNL_issue failed (input). Status = [0x%x]\n",
					status);
		}
		else
		{
			status = CHNL_reclaim(processorId,
			CHNL_ID_INPUT, WAIT_FOREVER, &CHNLGPE_IOReq);

			if (DSP_FAILED(status))
			{
				CHNLGPE_1Print("thrd_Dsplink_Recv: CHNL_reclaim failed (input). Status = [0x%x]\n",
						status);
			}
			else
			{
				nLength = AfxDataToInt16((unsigned char*) CHNLGPE_IOReq.buffer);
				if(nLength > CHNL_PACKET_LEN)
				{
					lgdebug(DBG_3,"thrd_Dsplink_Recv: dsplink data too long.\n");
					continue;
				}
				//CHNLGPE_1Print ("fffff :%x\n",CHNLGPE_IOReq.buffer[300]) ;
				calsChar = MakeSum((Uint8 *) CHNLGPE_IOReq.buffer, nLength);
				if (calsChar == CHNLGPE_IOReq.buffer[nLength - 1])
				{
					memcpy(g_dsplink_shm->rx_buffer, CHNLGPE_IOReq.buffer, nLength);
					g_dsplink_shm->rx_length = nLength;
					lgdebug(DBG_1,"thrd_Dsplink_Recv: dsplink data received.\n");
					/*post信号灯，交由协议处理*/
					sem_post(&g_dsplink_shm->dsplink_rx_sem);
					lgdebug(DBG_1,"thrd_Dsplink_Recv: dsplink_rx_sem posted.\n");
				}
			}
		}

	}
}

//Void thrd_Database()
//{
//
//		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);//���Ա��ر�
//		for(;;)
//		{
//
//		//		sem_wait(&sem);
//				//CHNLGPE_0Print ("new data\n") ;
//				thrd_Sql_Handle();		//数据库操作函数
//		}
//}

/** ============================================================================
 *  @func   CHNLGPE_Execute
 *
 *  @desc   This function implements the execute phase for this application.
 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
CHNLGPE_Execute (IN Uint32 numIterations, Uint8 processorId)
{
	DSP_STATUS status = DSP_SOK;
//	Uint32 i ,j;

	CHNLGPE_0Print ("CHNLGPE_Main: Entered CHNLGPE_Execute ()\n");
	/*
	 *  Start execution on DSP.
	 */
	status = PROC_start (processorId);

	if (DSP_SUCCEEDED (status))
	{
		CHNLGPE_IOReq.buffer = CHNLGPE_Buffers [0];
		CHNLGPE_IOReq.size = CHNLGPE_BufferSize;
		CHNLGPE_CMD_Send.buffer = CHNLGPE_Buffers [1];
		CHNLGPE_CMD_Send.size = CHNLGPE_BufferSize;

	}
	else
	{
		CHNLGPE_1Print ("CHNLGPE_Main: PROC_start failed. Status = [0x%x]\n", status);
		return status;
	}
	if (pthread_create(&tid_dsplink_senddaemon,NULL,thrd_Dsplink_SendDaemon,NULL)!=0)
	{
		CHNLGPE_0Print ("CHNLGPE_Main: creating sendDaemon thread failed!\n");
		return 1;
	}
	/*开始运行Dsplink接收主线程*/
	thrd_Dsplink_Recv();

	return status;
}





/** ============================================================================
 *  @func   CHNLGPE_Delete
 *
 *  @desc   This function releases resources allocated earlier by call to
 *          CHNLGPE_Create ().
 *          During cleanup, the allocated resources are being freed
 *          unconditionally. Actual applications may require stricter check
 *          against return values for robustness.
 *
 *  @modif  CHNLGPE_Buffers
 *  ============================================================================
 */
NORMAL_API
Void
CHNLGPE_Delete (Uint8 processorId)
{
    DSP_STATUS status    = DSP_SOK ;
    DSP_STATUS tmpStatus = DSP_SOK ;

    CHNLGPE_0Print ("CHNLGPE_Main: Entered CHNLGPE_Delete ()\n") ;

    /*
     *  Free the buffer(s) allocated for channel to DSP
     */
    tmpStatus = CHNL_freeBuffer (processorId,
                                 CHNL_ID_OUTPUT,
                                 CHNLGPE_Buffers,
                                 2) ;
    if (DSP_SUCCEEDED (status) && DSP_FAILED (tmpStatus)) {
        CHNLGPE_1Print ("CHNLGPE_Main: CHNL_freeBuffer () failed (output). Status = [0x%x]\n",
                     tmpStatus) ;
    }
		CHNLGPE_0Print ("CHNLGPE_Main: CHNL_freeBuffer () over ()\n") ;

    /*
     *  Delete both input and output channels
     */
    tmpStatus = CHNL_delete  (processorId, CHNL_ID_INPUT) ;
    if (DSP_SUCCEEDED (status) && DSP_FAILED (tmpStatus)) {
        CHNLGPE_1Print ("CHNLGPE_Main: CHNL_delete () failed (input). Status = [0x%x]\n",
                     tmpStatus) ;
    }
    tmpStatus = CHNL_delete  (processorId, CHNL_ID_OUTPUT) ;
    if (DSP_SUCCEEDED (status) && DSP_FAILED (tmpStatus)) {
        CHNLGPE_1Print ("CHNLGPE_Main: CHNL_delete () failed (output). Status = [0x%x]\n",
                     tmpStatus) ;
    }
				CHNLGPE_0Print ("CHNLGPE_Main: CHNL_delete  ()over\n") ;
    /*
     *  Stop execution on DSP.
     */
    status = PROC_stop (processorId) ;
    /*
     *  Close the pool
     */
    tmpStatus = POOL_close (POOL_makePoolId(processorId, POOL_ID)) ;
    if (DSP_SUCCEEDED (status) && DSP_FAILED (tmpStatus)) {
        CHNLGPE_1Print ("CHNLGPE_Main: POOL_close () failed. Status = [0x%x]\n",
                        tmpStatus) ;
    }
		CHNLGPE_0Print ("CHNLGPE_Main: POOL_close ()over\n") ;
    /*
     *  Detach from the processor
     */
    tmpStatus = PROC_detach  (processorId) ;
    if (DSP_SUCCEEDED (status) && DSP_FAILED (tmpStatus)) {
        CHNLGPE_1Print ("CHNLGPE_Main: PROC_detach () failed. Status = [0x%x]\n", tmpStatus) ;
    }
		CHNLGPE_0Print ("CHNLGPE_Main: PROC_detach () over\n") ;
    /*
     *  Destroy the PROC object.
     */
    tmpStatus = PROC_destroy () ;
    if (DSP_SUCCEEDED (status) && DSP_FAILED (tmpStatus)) {
        CHNLGPE_1Print ("CHNLGPE_Main: PROC_destroy () failed. Status = [0x%x]\n", tmpStatus) ;
    }
    CHNLGPE_0Print ("CHNLGPE_Main: Leaving CHNLGPE_Delete ()\n") ;
}


/** ============================================================================
 *  @func   CHNLGPE_Main
 *
 *  @desc   Entry point for the application
 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
Void *
CHNLGPE_Main (void *args)
{
    DSP_STATUS status       = DSP_SOK ;
    Uint8      processorId  = 0 ;
    LGPEPara  *para;
    para = (LGPEPara *)args;
//    signal(SIGINT, sigroutine);
//    signal(SIGTERM, sigroutine);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);/*线程可被结束*/
    CHNLGPE_0Print ("=============== LGPE Host Program : CHNLGPE_Main ==========\n") ;
    lgdebug(DBG_2, "CHNLGPE_Main paras: %s,%s,%s,%s;\n",
    		para->dspExecutable, para->strBufferSize, para->strNumIterations, para->processorId);
        if (processorId >= MAX_DSPS) {
            CHNLGPE_1Print ("==CHNLGPE_Main Error: Invalid processor id  specified %d ==\n",
                         processorId) ;
            status = DSP_EFAIL ;

        }
        /*
         *  Specify the dsp executable file name and the buffer size for
         *  chnlgpe creation phase.
         */
        if (DSP_SUCCEEDED (status)) {
             printf("====CHNLGPE_Main Executing %s for DSP processor Id %d ====\n",
            		 para->dspExecutable, processorId) ;
            status = CHNLGPE_Create (para->dspExecutable,
            							  para->strBufferSize,
                                     para->strNumIterations,
                                     processorId) ;
           /*
            *  Execute the data transfer chnlgpe.
            */
            if (DSP_SUCCEEDED (status)) {
            	printf("====CHNLGPE_Main start Executing ====\n");
            	CHNLGPE_Execute (0, 0);
            }

            /*
             *  Perform cleanup operation.
             */
            CHNLGPE_Delete (processorId) ;
  		}


    CHNLGPE_0Print ("====================================================\n") ;
    bExit = 1;
    pthread_exit(0);
}
