/*
 * dust_sensor.c
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "dust_sensor.h"
#include "../../peripheral/user_uart.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
#define MAX_DATA_SEND 7
#define MAX_DATA_RECIV 32
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define HEADER_01 0x42
#define HEADER_02 0x4D

/*
 * data header/power/mode/frequency/checksum
 * */
#define DUST_DATA_POSITION_HEADER_01 (00)
#define DUST_DATA_POSITION_HEADER_02 (01)
#define DUST_DATA_POSITION_LENGTH_H (02)
#define DUST_DATA_POSITION_LENGTH_L (03)
#define DUST_DATA_POSITION_PM25_H (12)
#define DUST_DATA_POSITION_PM25_L (13)
#define DUST_DATA_POSITION_PM10_H (14)
#define DUST_DATA_POSITION_PM10_L (15)
#define DUST_DATA_POSITION_VERSION (28)
#define DUST_DATA_POSITION_WARNING (29)

#define DUST_DATA_POSITION_CS_H (30)
#define DUST_DATA_POSITION_CS_L (31)

uint8_t DustSensorData[MAX_DATA_RECIV];

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/

static bool have_message_comming = false;
static void dust_com_process_task(void *pvParameters);
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
dust_error_t dust_error;

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

void dust_sensor_tx_process_task_creat(void)
{
	xTaskCreatePinnedToCore(dust_com_process_task, "dust_sensor_read", 1 * 1024, NULL, 5 | portPRIVILEGE_BIT, NULL, 1);
}

static void dust_com_process_task(void *pvParameters)
{
	APP_LOGD("dust_com_process_task call");
	uint32_t count_message = 0;
	while (1)
	{
		if (have_message_comming == false)
		{
			if (count_message % 10)
			{
				APP_LOGD("dust sensor disconnect");
			}
			count_message++;
		}
		else
		{
			count_message = 0;
			have_message_comming = false;
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
bool dust_sensor_parser_message(uint8_t *message, uint16_t length)
{
	bool status = true;

	if ((message[DUST_DATA_POSITION_HEADER_01] == HEADER_01) &&
		(message[DUST_DATA_POSITION_HEADER_02] == HEADER_02) &&
		(length == MAX_DATA_RECIV))
	{
		deive_data.sensor.pm25 = (message[DUST_DATA_POSITION_PM25_H] << 8) + message[DUST_DATA_POSITION_PM25_L];
		deive_data.sensor.pm10 = (message[DUST_DATA_POSITION_PM10_H] << 8) + message[DUST_DATA_POSITION_PM10_L];
		if (message[DUST_DATA_POSITION_WARNING])
		{
			deive_data.sensor.dust_error = true;
		}
		// APP_LOGD("message pm25 = %d", deive_data.sensor.pm25);
		// APP_LOGD("message pm10 = %d", deive_data.sensor.pm10);
	}
	else
	{
		APP_LOGE("message error lenght = %d", length);
		status = false;
	}
	return status;
}

/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/

static void vsm_protocol_send(uint8_t *buf, uint8_t buf_len)
{
	//UserUart_UartTX(AnyWay_Data.sum_data, MAX_DATA_SEND);
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
