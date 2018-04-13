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
*                                         BOARD SUPPORT PACKAGE
*
*                                      Texas Instruments TM4C129x
*                                                on the
*                                             DK-TM4C129X
*                                           Development Kit
*       		Modified by Dr. Samir A. Rawashdeh, for the TM4C123GH6PM microcontroller on the 
*						TM4C123G Tiva C Series Launchpad (TM4C123GXL), November 2014.
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               INCLUDES
*********************************************************************************************************
*/

#include  "..\bsp\bsp_sys.h"
#include  "..\bsp\bsp_led.h"
#include  "..\bsp\bsp_int.h"

// SAR Addition
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include <stdio.h>
/*$PAGE*/
/*
*********************************************************************************************************
*                                         BSP INITIALIZATION
*
* Description: This function should be called by the application code before using any functions in this
*              module.
*
* Arguments  : none
*********************************************************************************************************
*/

void  BSP_Init (void)
{		
		char str[10];		//temporary char array to print system clock to uart
	
    BSP_IntInit();
    BSP_SysInit();                                              /* Initialize system services, clocks, etc.             */
    BSP_LED_Init();
	
		// SAR: Initialize UART using TI's standard peripheral library
	
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
	
    GPIOPinConfigure(GPIO_PA1_U0TX);
	
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	
	
	/*
		SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinConfigure(GPIO_PD6_U2RX);
    GPIOPinConfigure(GPIO_PD7_U2TX);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
	
	*/
		UARTConfigSetExpClk(UART0_BASE,SysCtlClockGet(),115200,UART_CONFIG_WLEN_8);
	
		//UARTStdioConfig(0, 115200, BSP_SysClkFreqGet()); 
		//UARTStdioConfig(2, 9600, BSP_SysClkFreqGet()); 
	
		GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);
		GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_8MA,GPIO_PIN_TYPE_STD_WPU);
	/*
		UARTprintf("\nUART0 Init Complete.");
		UARTprintf("\nSystem clock = ");
  	sprintf(str,"%d",BSP_SysClkFreqGet());  
	  UARTprintf(str);
		UARTprintf("\n");
		*/
}

