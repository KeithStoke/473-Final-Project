/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           APPLICATION CODE
*
*                                      Texas Instruments TM4C129x
*                                                on the
*                                             DK-TM4C129X
*                                           Development Kit
*       		Modified by Dr. Samir A. Rawashdeh, for the TM4C123GH6PM microcontroller on the 
*						TM4C123G Tiva C Series Launchpad (TM4C123GXL), November 2014.
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
* Note(s)       : None.
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  "app_cfg.h"
#include  <cpu_core.h>
#include  <os.h>

#include  "..\bsp\bsp.h"
#include  "..\bsp\bsp_led.h"
#include  "..\bsp\bsp_sys.h"

// SAR Addition
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define SW1_PRESSED 0x01
#define SW2_PRESSED 0x02

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK       AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       Task1Stk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       Task2Stk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       Task3Stk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       SerialStk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       BeatStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_FLAG_GRP*		myGroup;
static  OS_EVENT* task1mail;
static  OS_EVENT* task2mail;

static  OS_EVENT* mySem;

static  OS_TCB* task1TCB;
static  OS_TCB* task2TCB;

/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskCreate         (void);
static  void  AppTaskStart          (void       *p_arg);
static  void  Task1          (void       *p_arg);
static  void  Task2          (void       *p_arg);
static  void  UpdateBlinkRate         (void       *p_arg);
static  void  SerialTerminal       (void       *p_arg);
static  void  HeartBeat      (void       *p_arg);

/*$PAGE*/
/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : (1) It is assumed that your code will call main() once you have performed all necessary
*                   initialization.
*********************************************************************************************************
*/

int  main (void)
{
	
#if (OS_TASK_NAME_EN > 0)
    CPU_INT08U  err;
#endif

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR     cpu_err;
#endif

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)"TM4C129XNCZAD",
                (CPU_ERR  *)&cpu_err);
#endif

    CPU_IntDis();                                               /* Disable all interrupts.                              */

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"          */

    OSTaskCreateExt((void (*)(void *)) AppTaskStart,           /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_START_PRIO,
                    (INT16U          ) APP_CFG_TASK_START_PRIO,
                    (OS_STK         *)&AppTaskStartStk[0],
                    (INT32U          ) APP_CFG_TASK_START_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_START_PRIO, "Start", &err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    while (1) {
        ;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           App_TaskStart()
*
* Description : Startup task example code.
*
* Arguments   : p_arg       Argument passed by 'OSTaskCreate()'.
*
* Returns     : none.
*
* Created by  : main().
*
* Notes       : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
		CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    (void)p_arg;                                                /* See Note #1                                              */


   (void)&p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */

    cpu_clk_freq = BSP_SysClkFreqGet();                         /* Determine SysTick reference freq.                    */
    cnts         = cpu_clk_freq                                 /* Determine nbr SysTick increments                     */
                 / (CPU_INT32U)OS_TICKS_PER_SEC;

    OS_CPU_SysTickInit(cnts);
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                                   */
#endif

    Mem_Init();

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
		
		
		myGroup = OSFlagCreate(0,0);
		
		task1mail = OSMboxCreate( (void* ) 0);
		task2mail = OSMboxCreate( (void* ) 0);
		
		mySem = OSSemCreate(1);
		
		BSP_LED_Toggle(0);
		OSTimeDlyHMSM(0, 0, 0, 200);
		BSP_LED_Toggle(0);
		BSP_LED_Toggle(1);
		OSTimeDlyHMSM(0, 0, 0, 200);
		BSP_LED_Toggle(1);
		BSP_LED_Toggle(2);
		OSTimeDlyHMSM(0, 0, 0, 200);    
		BSP_LED_Toggle(2);

		OSTimeDlyHMSM(0, 0, 1, 0);   

		AppTaskCreate();                                            /* Creates all the necessary application tasks.         */

    while (DEF_ON) {

        OSTimeDlyHMSM(0, 0, 0, 200);			

    }
}


/*
*********************************************************************************************************
*                                         AppTaskCreate()
*
* Description :  Create the application tasks.
*
* Argument(s) :  none.
*
* Return(s)   :  none.
*
* Caller(s)   :  AppTaskStart()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
OSTaskCreate((void (*)(void *)) Task1,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&Task1Stk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 5 );  						// Task Priority
                

OSTaskCreate((void (*)(void *)) UpdateBlinkRate,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&Task2Stk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 6 );  						// Task Priority
 
OSTaskCreate((void (*)(void *)) Task2,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&Task3Stk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 7 );  						// Task Priority	
										
OSTaskCreate((void (*)(void *)) HeartBeat,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&BeatStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 9 );  						// Task Priority	
																		
OSTaskCreate((void (*)(void *)) SerialTerminal,           /* Create the second task                                */
                    (void           *) 0,							// argument
                    (OS_STK         *)&SerialStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) 10 );  						// Task Priority												
}


static  void  Task1 (void *p_arg)
{
		INT32U rate = 400;
   (void)p_arg;
	
		task1TCB = OSTCBCur;
    while (1) {              
        BSP_LED_Toggle(1);
				UARTprintf("T1 ");	// Probably needs to be protected by semaphore
				if( OSFlagAccept(myGroup,SW1_PRESSED,OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME,0) == SW1_PRESSED )
				{
					rate = rand() % 1000;
					OSMboxPost(task1mail,(void*)&rate);
					//UARTprintf("\n Task1 New Rate = %d\n",rate);
				}
        OSTimeDlyHMSM(0, 0, 0, rate);
			}
}

static  void  Task2 (void *p_arg)
{
		INT32U rate = 400;
   (void)p_arg;
	  task2TCB = OSTCBCur;
    while (1) {              
        BSP_LED_Toggle(2);
  			UARTprintf("T2 ");  // Probably needs to be protected by semaphore
			
			if( OSFlagAccept(myGroup,SW1_PRESSED,OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME,0) == SW1_PRESSED )
				{
					rate = rand() % 1000;
					OSMboxPost(task2mail,(void*)&rate);
					//UARTprintf("\nTask2 New Rate = %d\n",rate);
				}
				
        OSTimeDlyHMSM(0, 0, 0, rate);
			}
}

static  void  UpdateBlinkRate (void *p_arg)
{
   (void)p_arg;
	
    while (1) {              
        
			if( !GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
				{
					//UARTprintf("T3 ");  // Probably needs to be protected by semaphore
					OSFlagPost(myGroup,SW1_PRESSED,OS_FLAG_SET,0);
				}
			if( !GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))
				{
					//UARTprintf("T3 ");  // Probably needs to be protected by semaphore
					OSFlagPost(myGroup,SW2_PRESSED,OS_FLAG_SET,0);
				}
				
        OSTimeDlyHMSM(0, 0, 0, 100);
			}
}

static  void  HeartBeat(void *p_arg)
{
   (void)p_arg;
	
    while (1) {
				OSSemPend(mySem,0,0);
        UARTprintf("\n Pulse \n");
				OSSemPost(mySem);
        OSTimeDlyHMSM(0, 0, 0, 500);
			}
}

static  void  SerialTerminal (void *p_arg)
{
		int* task1rate;
		int* task2rate;
		
		int test=0;
   (void)p_arg;
		
    while (1) {              
        
			task1rate = (int*)OSMboxAccept(task1mail);
				
			task2rate = (int*)OSMboxAccept(task2mail);
		
			if ( task1rate != (void*) 0 )
			{
				OSSemPend(mySem,0,0);
				UARTprintf("\nTask1 New Rate = %d\n",*task1rate);
				OSSemPost(mySem);
			}
			
			
			
			if ( task2rate != (void*) 0 )
			{
				OSSemPend(mySem,0,0);
				UARTprintf("\nTask2 New Rate = %d\n",*task2rate);
				OSSemPost(mySem);
			}
			
			
			if ( task1rate == (void*) 0 && task2rate == (void*) 0 )
			{
				OSSemPend(mySem,0,0);
				test = OSCPUUsage;
				UARTprintf("\n\n CPU Usage: %d percent\n Context Switches: %d \n Idle Counter: %d \n Times Blinky1 Switched: %d \n Times Blinky2 Switched: %d\n\n",test,OSCtxSwCtr,OSIdleCtr,task1TCB->OSTCBCtxSwCtr,task2TCB->OSTCBCtxSwCtr );
				OSSemPost(mySem);
			}
			OSTimeDlyHMSM(0, 0, 0, 1000);
			}
}
