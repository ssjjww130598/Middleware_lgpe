/******************************************************************************
版权信息 : 版权所有 (C), 2011-2014, 理工电力电子设备有限公司
工 程 名 : chnlgpe_v1.3
文 件 名 : main.c
创建日期 : 2014年3月14日
文件说明 :

修改历史 :
REV1.0.0  Chase  2014年3月14日  文件创建

******************************************************************************
Copyright    : LGPE Co.,Ltd. All Rights Reserved.
Project Name : chnlgpe_v1.3
File Name    : main.c
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
#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

#define DSPLINK_DATAIN_HANDLER
//#define CHNLGPE_THREAD_TEST

/*----------------------------------------------------------*
 * 局部结构定义       Local Structure Define Section        	*
 *----------------------------------------------------------*/

#if defined (DA8XXGEM)

/** ============================================================================
 *  @name   dspAddr
 *
 *  @desc   Address of c_int00 in the DSP executable.
 *  ============================================================================
 */
extern Uint32 CHNLGPE_dspAddr ;

/** ============================================================================
 *  @name   shmAddr
 *
 *  @desc   Address of symbol DSPLINK_shmBaseAddres in the DSP executable.
 *  ============================================================================
 */
extern Uint32 CHNLGPE_shmAddr ;

/** ============================================================================
 *  @name   argsAddr
 *
 *  @desc   Address of .args section in the DSP executable.
 *  ============================================================================
 */
extern Uint32 CHNLGPE_argsAddr ;

/** ============================================================================
 *  @name   LINKCFG_config
 *
 *  @desc   Extern declaration to the default DSP/BIOS LINK configuration
 *          structure.
 *  ============================================================================
 */
extern  LINKCFG_Object LINKCFG_config ;
#endif


/*----------------------------------------------------------*
 * 全局变量声明         Global Variable Declare Section     *
 *----------------------------------------------------------*/
pthread_t tid1;
pthread_t tid2;
pthread_t tid3;
pthread_mutex_t mutex;/*线程锁，控制socket链接链表*/
pthread_mutex_t sql_mutex;/*线程锁，控制sql处理队列*/
sem_t sem;				/*数据库队列操作信号量*/
int pid_dsplink;
volatile int bExit = 0;
volatile int g_dsplinkExit = 0;
unsigned char *strProtocol;
extern cJSON *g_jsonCmdConfig;

/*----------------------------------------------------------*
 * 局部函数原型声明   Local Prototype Declare Section       	*
 *----------------------------------------------------------*/
int chnlgpe_test_nofork(int argc, char ** argv);
/*----------------------------------------------------------*
 * 函数定义           Function Define Section               *
 *----------------------------------------------------------*/
void sigroutine(int dunno)
{
	if (dunno == 2)
	{
		printf("Process[%d]:Get a signal -- SIGINT\n", getpid());
		bExit = 1;
		g_dsplinkExit = 1;
	}
	if (dunno == 15)
	{
		printf("Process[%d]:Get a signal -- SIGTERM\n", getpid());
		bExit = 1;
		g_dsplinkExit = 1;
	}
	return;
}

void send_test_frame_to_dsp()
{

}
int chnlgpe_initialize()
{

	/*读取json配置文件*/
	if (LGPE_ConfigReadFromFile() == -1)
		return -1;
	/*互斥锁初始化*/
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);		/*设置为进程间共享*/
	pthread_mutex_init(&mutex, &attr);
	pthread_mutex_init(&sql_mutex, &attr);

	/*发送字符串分配内存*/
	strProtocol = malloc(sizeof(unsigned char) * 1000);

	/*初始化socket模块*/
	lgpe_Socket_Initialize();
	/*初始化数据库操作队列*/
	g_sqlOperate_list = LGPE_sql_operate_Create_List();
	/*信号量初始化*/
	sem_init(&sem,0,0);
}

void chnlgpe_finalize()
{
	lgpe_Socket_Finalize(); /*socket资源回收*/
	free(strProtocol); /*发送字符串内存回收 */
	cJSON_Delete(g_jsonCmdConfig); /*cmdjson结构体回收 */
	LGPEConfig_remove_list(g_jsonConfigList);/*json配置文件列表回收 */
	pthread_mutex_destroy(&mutex);/*互斥锁销毁*/
	pthread_mutex_destroy(&sql_mutex);/*互斥锁销毁*/
	LGPE_sql_operate_remove_list(g_sqlOperate_list);/*初始化数据库操作移除*/
	sem_destroy(&sem);/*信号量销毁*/
}
/** ============================================================================
 *  @func   main
 *
 *  @desc   Entry point for the application
 *
 *  @modif  None
 *  ============================================================================
 */
int main (int argc, char ** argv)
{
	/*跟踪内存分配情况*/
	setenv("MALLOC_TRACE", "chnlgpe_v1.3_mtrace.log", 1);		/*设置内存分配记录文件名*/
//	mtrace();
    Char8 * dspExecutable    = NULL ;
    Char8 * strBufferSize    = NULL ;
    Char8 * strNumIterations = NULL ;
    Char8 * strProcessorId   = NULL ;
    Uint8 processorId        = 0    ;
#if defined (DA8XXGEM)
    Char8 * strDspAddr       = NULL ;
    Char8 * strShmAddr       = NULL ;
    Char8 * strArgsAddr      = NULL ;
#endif

#if 0
	LINKCFG_config.dspConfigs[processorId]->dspObject->doDspCtrl =	DSP_BootMode_NoBoot;
	strcpy(LINKCFG_config.dspConfigs [processorId]->dspObject->loaderName, "NOLOADER");
#endif
	
	lgdebug_initialize("log.txt");
	/*设置调试输出等级为DGB_2*/
	lgdebug_set_level_s("DBG_2");
	lgdebug(DBG_INFO,"Main:lgdebug initialized!\t\n");
    if ((argc != 5) && (argc != 4) && (argc != 2)) {
        printf ("Usage : %s <absolute path of DSP executable> "
           "<Buffer Size> <number of transfers> < DSP Processor Id >\n"
           "For infinite transfers, use value of 0 for <number of transfers>\n"
           "For DSP Processor Id,"
           "\n\t use value of 0  if sample needs to be run on DSP 0 "
           "\n\t use value of 1  if sample needs to be run on DSP 1"
           "\n\t For single DSP configuration this is optional argument\n",
           argv [0]) ;
    }
    else
    {
    	/*初始化dsplink共享内存*/
//    	lgpe_dsplink_shm_init(g_dsplink_shm);
    	g_dsplink_shm = lgpe_dsplink_shm_create("dsplink_shm");
    	lgdebug(DBG_2, "Main: g_dsplink_shm inited. - %x.\t\n", (unsigned int)g_dsplink_shm);

		/*信号拦截*/
		signal(SIGINT, sigroutine);
		signal(SIGTERM, sigroutine);
		printf("Main[%d]: trapping the signal 0x%x.\n", getpid(),(unsigned int)sigroutine);

//		chnlgpe_test_nofork();				/*单进程多线程测试代码*/

		/**/
		pid_dsplink = fork();
		/*启动线程*/
		if (pid_dsplink > 0)
		{
			/*============================主进程=============================*/
			if(chnlgpe_initialize()==-1)
			{
				CHNLGPE_1Print("read json config file failed please kill%d\n",pid_dsplink);
				return 0;
			}
			/*启动数据库线程*/
			if (pthread_create(&tid1, NULL, thrd_Database, NULL) != 0) //database thread
			{
				CHNLGPE_0Print("Main:Create database thread error!\n");
				chnlgpe_finalize();
				return 0;
			}
			else
			{
				CHNLGPE_1Print("Main:Create database thread %d !\n", tid1);
			}
			/*启动socketRecv线程*/
			if (pthread_create(&tid2, NULL, lgpe_Socket_Recv_Thread, NULL) != 0) //socket thread
			{
				CHNLGPE_0Print("Main:Create socket recv thread error!\n");
				pthread_cancel(tid1);
				pthread_join(tid1, NULL);
				chnlgpe_finalize();
				return 0;
			}
			else
			{
				CHNLGPE_1Print("Main:Create socket recv thread %d !\n", tid2);
			}
#ifdef DSPLINK_DATAIN_HANDLER
			/*创建dsplink接收处理线程*/
			if (pthread_create(&tid3, NULL, lgpe_protocol_datain_handler, NULL) != 0) /*create dsplink thread*/
			{
				CHNLGPE_0Print("Main: Create lgpe_protocol_datain_handler thread error!\n");
				pthread_cancel(tid1);
				pthread_cancel(tid2);
				pthread_join(tid1, NULL);
				pthread_join(tid2, NULL);
				chnlgpe_finalize();
				return 0;
			}
			else
			{
				CHNLGPE_1Print("Main: Created lgpe_protocol_datain_handler thread %d !\n",tid3);
			}
#endif
			for (;;)
			{
				usleep(200000);
				/*判断是否被中断*/
				if (bExit == 1)
				{
					CHNLGPE_0Print("Main: Exiting...\n");
					lgdebug(DBG_2, "Main: Exiting...\n");
					/*if (pthread_cancel(tid1)==0)
					 {
					 CHNLGPE_0Print ("Send Cancel cmd to dsplink Thread\n");
					 }*/

					/*终止socket接收处理线程*/
					if (pthread_cancel(tid2) == 0)
					{
						printf("Main[%d]:Send Cancel cmd to socket Thread.\n", getpid
								());
					}
#ifdef DSPLINK_DATAIN_HANDLER
					/*终止dsplink接收处理线程*/
					if (pthread_cancel(tid3) == 0)
					{
						printf(
								"Main[%d]:Send Cancel cmd to dsplink handler Thread.\n", getpid
								());
					}

#endif
#if 1
			//		if (pthread_cancel(tid3)==0)  // 关闭tid3
			//		{
			//				CHNLGPE_0Print ("Send Cancel cmd to database Thread\n");
			//		}
					chnlgpe_finalize();

//					kill(-pid_dsplink, SIGTERM);
//					printf("Main: killing dsplink process.\n");
#endif
					/*等待线程结束 added by chase 20140318*/
					pthread_join(tid2, NULL);
					pthread_join(tid3, NULL);

					int status;
					wait(&status);
					if(status != 0)
						printf("Main[%d]: son process return with error.\n", getpid());
					printf("Main[%d]: exited\n", getpid());
					return 0;
				}
			}
		}
		else if(pid_dsplink == 0)
		{
			/*==========================DSP/Link进程==============================*/
			lgdebug(DBG_INFO,"Dsp/Link thread created.\t\n");

			/*dsplnk参数初始化*/
			strBufferSize = "1024";
			strNumIterations = "10";
			/*生成CHNLGPE_Main参数*/
			dspExecutable = argv[1];
			if (argc > 2)
			{
				strBufferSize = argv[2];
				strNumIterations = argv[3];
			}

			if (argc == 4)
			{
				strProcessorId = "0";
				processorId = 0;
			}
			else
			{
				strProcessorId = argv[4];
				processorId = atoi(argv[4]);
			}
			//		signal(SIGINT, sigroutine);
			if (processorId < MAX_PROCESSORS)
			{
#if defined (DA8XXGEM)
				if ( LINKCFG_config.dspConfigs [processorId]->dspObject->doDspCtrl
						== DSP_BootMode_NoBoot)
				{
					/* strDspAddr(c_int00 address)  and .args address are not required
					 * for noboot mode.DSPLINK_shmBaseAddress address is not required to
					 * pass for  message sample. Because  DSPLINK_shmBaseAddress is
					 * defined in linker command file of  dsp side message sample.
					 */
					CHNLGPE_0Print("Start Running in NoBoot Mode!\n");
					strShmAddr = "0x0";
					strDspAddr = "0x0";
					strArgsAddr = "0x0";
					CHNLGPE_shmAddr = CHNLGPE_Atoll (strShmAddr);
					CHNLGPE_dspAddr = CHNLGPE_Atoll (strDspAddr);
					CHNLGPE_argsAddr = CHNLGPE_Atoll (strArgsAddr);
					/* For No bootmode Hard coding the values
					 * since DSP side app is using the same values
					 */
					strBufferSize = "1024";
					strNumIterations = "10000";

				}
#endif
				LGPEPara para;
				para.dspExecutable = dspExecutable;
				para.strBufferSize = strBufferSize;
				para.strNumIterations = strNumIterations;
				para.processorId = "0";
//				CHNLGPE_Main((void*)&para);
#if 1
				if (pthread_create(&tid1, NULL, CHNLGPE_Main, (void *) &para)
						!= 0) /*create dsplink thread*/
				{
					printf("CHNLGPE_Main[%d]:Create dsplink thread error!\n",getpid());
					return 0;
				}
				else
				{
					printf("CHNLGPE_Main[%d]:Create dsplink thread!\n",getpid());
				}

				for (;;)
				{
					usleep(200000);
					/*判断是否被中断*/
					if (bExit == 1)
					{
						printf("CHNLGPE_Main[%d]: killing dsplink process.\n", getpid());
						pthread_join(tid1, NULL);
						return 0;
					}
				}
#endif
			}
		}
		else
		{
			/*创建进程失败*/
			perror("Main: fork() failed!\n");
			exit(-1);
		}
	}
	return 0;
}

int chnlgpe_test_nofork(int argc, char ** argv)
{
#ifdef CHNLGPE_THREAD_TEST
		/*==========================DSP/Link进程==============================*/

//
//		lgdebug(DBG_INFO,"Dsp/Link thread created.\t\n");

		/*dsplnk参数初始化*/
		char *strBufferSize = "1024";
		char *strNumIterations = "10";
		/*生成CHNLGPE_Main参数*/
		char *dspExecutable = argv[1];
		char *strProcessorId = NULL;
		Uint8 processorId        = 0    ;

		if (argc > 2)
		{
			strBufferSize = argv[2];
			strNumIterations = argv[3];
		}

		if(argc == 4 ) {
			strProcessorId   = "0" ;
			processorId      = 0 ;
        }
       else {
    	   strProcessorId   = argv [4] ;
    	   processorId      = atoi (argv [4]) ;
        }
//		signal(SIGINT, sigroutine);
		if (processorId < MAX_PROCESSORS)
		{
			LGPEPara para;
			para.dspExecutable = dspExecutable;
			para.strBufferSize = strBufferSize;
			para.strNumIterations = strNumIterations;
			para.processorId = "0";
//			CHNLGPE_Main((void *) &para);

			lgpe_Socket_Initialize();

			if (pthread_create(&tid1, NULL, CHNLGPE_Main, (void *) &para)
									!= 0) /*create dsplink thread*/
			{
				printf("CHNLGPE_Main[%d]:Create dsplink thread error!\n",getpid());
				return 0;
			}
			else
			{
				printf("CHNLGPE_Main[%d]:Create dsplink thread!\n",getpid());
			}
			if (pthread_create(&tid2, NULL, lgpe_Socket_Recv_Thread, NULL) != 0) //socket thread
			{
				CHNLGPE_0Print("Main:Create socket recv thread error!\n");
				//				pthread_cancel(tid1);
				pthread_join(tid1, NULL);
				lgpe_Socket_Finalize(); /*socket资源回收*/
				free(strProtocol); /*发送字符串内存回收 */
				cJSON_Delete(g_jsonCmdConfig); /*cmdjson结构体回收 */
				//		sem_destroy(&sem);
				return 0;
			}
			else
			{
				CHNLGPE_0Print("Main:Create socket recv thread!\n");
			}
#ifdef DSPLINK_DATAIN_HANDLER
			/*创建dsplink接收处理线程*/
			if (pthread_create(&tid3, NULL, lgpe_protocol_datain_handler, NULL)
					!= 0) /*create dsplink thread*/
			{
				CHNLGPE_0Print(
						"Main: Create lgpe_protocol_datain_handler thread error!\n");
				return 0;
			}
			else
			{
				CHNLGPE_0Print(
						"Main: Create  lgpe_protocol_datain_handler thread!\n");
			}
#endif
			for (;;)
			{
				usleep(200000);
				/*判断是否被中断*/
				if (bExit == 1)
				{
					CHNLGPE_0Print("Main: Exiting...\n");
					lgdebug(DBG_2, "Main: Exiting...\n");
					if (pthread_cancel(tid1) == 0)
					{
						CHNLGPE_0Print("Send Cancel cmd to dsplink Thread\n");
					}

					/*终止socket接收处理线程*/
					if (pthread_cancel(tid2) == 0)
					{
						printf("Main[%d]:Send Cancel cmd to socket Thread.\n",
								getpid());
					}
#ifdef DSPLINK_DATAIN_HANDLER
					/*终止dsplink接收处理线程*/
					if (pthread_cancel(tid3) == 0)
					{
						printf(
								"Main[%d]:Send Cancel cmd to dsplink handler Thread.\n",
								getpid());
					}
#endif
#if 1
					lgpe_Socket_Finalize(); /*socket资源回收*/

//					kill(-pid_dsplink, SIGTERM);
//					printf("Main: killing dsplink process.\n");
					/*等待线程结束 added by chase 20140318*/
#endif
					pthread_join(tid1, NULL);
					pthread_join(tid2, NULL);
					pthread_join(tid3, NULL);

					int status;
					wait(&status);
					if(status != 0)
						printf("Main[%d]: son process return with error.\n", getpid());
					printf("Main[%d]: exited\n", getpid());
					return 0;
				}
			}
		}
#endif
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
