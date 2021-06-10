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
#include "../user_driver/dust_sensor/dust_sensor.h"

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
static void state_check_connect_reset(void);

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

void uart_dust_sensor_init(void)
{
	UserUart_2_Init(UART_BAUD_9600);
}
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
void uart_dust_sensor_start(void)
{
	xTaskCreate(uart_rx_task, "uart_rx_task", 3 * 1024, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

void uart_dust_sensor_send(unsigned char *buf, unsigned char size)
{
	UserUart_WriteData(buf, size);
}

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
uint8_t state_check_connection = 0;
uint32_t time_count_previous = 0;
uint32_t count_message_error = 0;
static void uart_rx_task(void *pvParameters)
{
	uint8_t data[512] = {0};
	unsigned char Rx_len = 0;
	vTaskDelay(1000);
	printf("\r\n---- dust_sensor read ");
	// dust_sensor_tx_process_task_creat();

	while (1)
	{
		if ((Rx_len = UserUart_ReadData(data)) > 0)
		{
			bool status = dust_sensor_parser_message((uint8_t *)data, Rx_len);
			memset(data, 0x00, sizeof(data));
			Rx_len = 0;
			state_check_connect_reset();
			// update status data
			if (status == true)
			{
				deive_data.sensor.dust_error = false;
				count_message_error = 0;
			}
			else if (status == false)
			{
				count_message_error++;
				if (count_message_error > 5)
				{
					deive_data.sensor.dust_error = true;
					count_message_error = 0;
					// reset data pm
					deive_data.sensor.pm10 = 0;
					deive_data.sensor.pm25 = 0;
				}
			}
		}
		else
		{
			switch (state_check_connection)
			{
			case 0:
				time_count_previous = xTaskGetTickCount();
				state_check_connection = 1;
				break;
			case 1:
				if ((xTaskGetTickCount() - time_count_previous) > 1000) // check after 10000s
				{
					APP_LOGD("dust sensor lost connection");
					state_check_connect_reset();
					deive_data.sensor.dust_error = true;
					// reset dust data to 0
					deive_data.sensor.pm10 = 0;
					deive_data.sensor.pm25 = 0;
				}
				break;
			default:
				break;
			}
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}
/***********************************************************************************************************************
* static functions
***********************************************************************************************************************/
static void state_check_connect_reset(void)
{
	state_check_connection = 0;
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
