/*
 * uart_task.c
 *
 *  Created on: Nov 24, 2020
 *      Author: Yolo
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "uart_task.h"
#include "../peripheral/user_uart.h"
#include "../user_driver/stm32_com/stm32_com.h"

/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void uart_rx_task(void *pvParameters);

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

void uart_com2stm32_init(void)
{
	UserUart_2_Init(UART_BAUD_115200);
}
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
void uart_com2stm32_start(void)
{
	xTaskCreate(uart_rx_task, "uart_rx_task", 3 * 1024, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

void uart_com2stm32_send(unsigned char* buf, unsigned char size)
{
	UserUart_WriteData(buf, size);
}

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void uart_rx_task(void *pvParameters)
{
	uint8_t data[512] = {0};
	unsigned char Rx_len = 0;
	vTaskDelay(1000);
	printf("\r\n---- Uart_Task ");
	anywave_tx_process_task_creat();
	while(1)
	{
		if((Rx_len = UserUart_ReadData(data)) > 0)
		{
			anywave_parser_message((uint8_t*)data , Rx_len);
			memset(data, 0x00, sizeof(data));
			Rx_len = 0;
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}
/***********************************************************************************************************************
* static functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
