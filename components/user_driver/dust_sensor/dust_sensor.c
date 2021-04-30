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

uint8_t DustSensorData[MAX_DATA_RECIV];

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/


static bool have_message_comming = false;
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/


/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/


void anywave_tx_process_task_creat(void)
{
	xTaskCreatePinnedToCore(anywave_com_process_task, "anywave_com", 2 * 1024, NULL, 5 | portPRIVILEGE_BIT, NULL, 1);
}

static void anywave_com_process_task(void *pvParameters)
{
	APP_LOGD("anywave_com_process_task call");
	uint32_t count_message = 0;
	while (1)
	{
		anywave_com_process();
		if( have_message_comming == false)
		{
			if( count_message % 10)
			{
				APP_LOGD("Connection error");
			}
			count_message++;
		}
		else
		{
			count_message = 0;
			have_message_comming = false;
		}
		APP_LOGD("Connection error 2");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void anywave_set_event(e_esp32stm32_com event)
{
	esp32stm32_com = event;
}
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void anywave_com_process( void )
{
	switch (esp32stm32_com)
	{
		case E_ESP_NONE:
			break;
		case E_ESP_GET_STATE:
			vsm_protocol_device_response(E_COMMAND_GET_STATE);
			esp32stm32_com = E_ESP_SET_STATE;
			break;
		case E_ESP_SET_STATE:
			vsm_protocol_device_response(E_COMMAND_SET_STATE);
			esp32stm32_com = E_ESP_GET_STATE;
			break;
		case E_STM32_RESPONSE:
			break;
		case E_STM32_TIMEOUT:
			break;
		default:
			break;
	}
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
bool anywave_parser_message(uint8_t *message, uint16_t length)
{
	have_message_comming = true;
	bool status = false;
	AnyWay_Data_t _AnyWay_Data;
	if( length == MAX_DATA_SEND)
	{
		for (int i = 0; i < length; i++)
		{
			_AnyWay_Data.sum_data[i] = message[i];
			APP_LOGD("data = %x", _AnyWay_Data.sum_data[i]);
		}
		if( _AnyWay_Data.command == E_COMMAND_RESPONSE)
		{
			device_data.device_state.error = _AnyWay_Data.error; // get only error state from stm32
			APP_LOGD("_AnyWay_Data.error %d", _AnyWay_Data.error);
		}
		APP_LOGD("Command %d", _AnyWay_Data.command);
	}
	else
	{
		status = false;
		APP_LOGE("error length");
	}

	return status;
}

static void vsm_protocol_device_response(e_anywave_command command)
{

}

/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/

static void vsm_protocol_send(uint8_t *buf, uint8_t buf_len)
{

    UserUart_UartTX(AnyWay_Data.sum_data, MAX_DATA_SEND);
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/

