/*
    Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
    Copyright (c) 2012 - 2020 Xilinx, Inc. All Rights Reserved.
	SPDX-License-Identifier: MIT


    http://www.FreeRTOS.org
    http://aws.amazon.com/freertos


    1 tab == 4 spaces!
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"

#include "xgpiops.h"

#define TASK_A_PRIORITY				(tskIDLE_PRIORITY + 1)

#define TASK_A_STACK_SIZE			(1024)	// 1024*4

#define D23_LED						10

XGpioPs psGpioInstance;

void ExtIrq_Handler(void *InstancePtr)
{
	xil_printf("ExtIrq_Handler\r\n");
}

void interrupt_init()
{
	xPortInstallInterruptHandler(XPAR_FABRIC_EXT_IRQ_INTR, ExtIrq_Handler, (void *)NULL);
	vPortEnableInterrupt(XPAR_FABRIC_EXT_IRQ_INTR);
}

void led_toggle()
{
	u32 pin = XGpioPs_ReadPin(&psGpioInstance, D23_LED);
	pin ^= 1;
	XGpioPs_WritePin(&psGpioInstance, D23_LED, pin);
}

u8 gpio_init()
{
	int xStatus;

	u32 uLedDirection = 0x1;		// Output

	XGpioPs_Config *GpioConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	if(GpioConfigPtr == NULL) {
		xil_printf("Error : XGpioPs_LookupConfig(%s:%d)\r\n", __FILE__, __LINE__);
		return FALSE;
	}
	xStatus = XGpioPs_CfgInitialize(&psGpioInstance, GpioConfigPtr, GpioConfigPtr->BaseAddr);
	if(XST_SUCCESS != xStatus) {
		xil_printf("Error : XGpioPs_CfgInitialize(%s:%d)\r\n", __FILE__, __LINE__);
		return FALSE;
	}

	XGpioPs_SetDirectionPin(&psGpioInstance, D23_LED, uLedDirection);
	XGpioPs_SetOutputEnablePin(&psGpioInstance, D23_LED, 1);

	return TRUE;
}

u8 sys_init()
{
	if(!gpio_init()) {
		xil_printf("Error : gpio_init(%s:%d)\r\n", __FILE__, __LINE__);
		return FALSE;
	}

	interrupt_init();

	return TRUE;
}

static void TaskA(void *pvParameters)
{
	while(1) {
		led_toggle();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

int main( void )
{
	xil_printf("Hello FreeRTOS.\r\n");

	if(!sys_init()) {
		goto out;
	}

	xTaskCreate(TaskA, "TaskA", TASK_A_STACK_SIZE, NULL, TASK_A_PRIORITY, (TaskHandle_t *) NULL);

	vTaskStartScheduler();

out:
	for( ;; );
}
