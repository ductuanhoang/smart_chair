/*
 * json_parser.c
 *
 *  Created on: Nov 16, 2020
 *      Author: Yolo
 */

/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "json_parser.h"
#include "cJson_lib/cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "../Interface/Logger_File/logger_file.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
/*
{
  "type": "anywave",
  "command":"control",
  "payload": {
    "Power": 1,
    "Mode": 0,
    "Frequency": 300
  }
}
 * */
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/

#define TYPE_COMMAND_CONTROL "control"
#define TYPE_COMMAND_PAYLOAD "payload"
#define TYPE_COMMAND_SMART_CONFIG "smart_config"

#define TEST 1
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static bool json_parser_command_contrl(const char *message);
static bool json_parser_command_smart_config(const char *message);
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
bool json_parser_message(const char *message, uint8_t source)
{
	APP_LOGD("Serialize.....");
	APP_LOGI("message = %s", message);

	bool status = false;
	cJSON *root2 = cJSON_Parse(message);
	cJSON *command = cJSON_GetObjectItem(root2, "command");
	if (command)
	{
		char *value_type_cmd = cJSON_GetObjectItem(root2, "command")->valuestring;
		if ((strcmp(value_type_cmd, TYPE_COMMAND_CONTROL) == 0))
		{
			status = json_parser_command_contrl(message);
//			DeviceData.new_ctrl_cmd = E_CMD_CTRL;
		}
		else if ((strcmp(value_type_cmd, TYPE_COMMAND_SMART_CONFIG) == 0))
		{
			status = json_parser_command_smart_config(message);
		}
	}
	else
	{
		APP_LOGD("thing_token have'nt");
		status = false;
	}

	cJSON_Delete(root2);

	return status;
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
char *json_packet_message(char *cmd, bool status_response)
{
	cJSON *root = NULL;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "type", cJSON_CreateString("device_response"));
	cJSON_AddNumberToObject(root, "code", status_response);
	cJSON_AddStringToObject(root, "trans_code", cmd);
	//	return out = cJSON_Print(root);
	APP_LOGD("message = %s", cJSON_PrintUnformatted(root));
	return cJSON_PrintUnformatted(root);
}

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static bool json_parser_command_contrl(const char *message)
{
	bool status = false;
	cJSON *root2 = cJSON_Parse(message);
	cJSON *Payload = cJSON_GetObjectItem(root2, TYPE_COMMAND_PAYLOAD);

	if (Payload)
	{
		status = true;
		uint16_t type_power = cJSON_GetObjectItem(Payload, "Power")->valueint;
		uint16_t type_mode = cJSON_GetObjectItem(Payload, "Mode")->valueint;
		uint16_t typed_level =  cJSON_GetObjectItem(Payload, "level")->valueint;
		uint16_t typed_uv_led =  cJSON_GetObjectItem(Payload, "uv_led")->valueint;

		APP_LOGI("type_power = %d", type_power);
		APP_LOGI("type_mode = %d", type_mode);
		APP_LOGI("typed_level = %d", typed_level);
		APP_LOGI("typed_uv_led = %d", typed_uv_led);

		device_data.device_state.power = type_power;
		device_data.device_state.mode = type_mode;
		device_data.device_state.uv_led = typed_uv_led;

		// limited value for level
		if(typed_level > E_INTENSITY_ULTRA)
		{
			typed_level = E_INTENSITY_ULTRA;
		}

		device_data.device_state.level = typed_level;
	}
	else
	{
		APP_LOGE("parser command control err");
	}

	cJSON_Delete(root2);
	return status;
}


static bool json_parser_command_smart_config(const char *message)
{
	bool status = false;
	cJSON *root2 = cJSON_Parse(message);
	cJSON *wifi_name = cJSON_GetObjectItem(root2, "wifi_ssid");
	cJSON *wifi_password = cJSON_GetObjectItem(root2, "wifi_password");

	if (wifi_name && wifi_password)
	{
		char *type_wifi_name = cJSON_GetObjectItem(root2, "wifi_ssid")->valuestring;
		char *type_wifi_password = cJSON_GetObjectItem(root2, "wifi_password")->valuestring;
		APP_LOGI("type_wifi_name = %s", type_wifi_name);
		APP_LOGI("type_wifi_password = %s", type_wifi_password);

		sprintf(wifi_info.wifi_name, "%s", type_wifi_name);
		sprintf(wifi_info.wifi_pass, "%s", type_wifi_password);

		// logger_LoadInfo_WifiStation(true);
		status = true;
	}
	else
	{
		APP_LOGE("parser smart config err");
	}

	cJSON_Delete(root2);
	return status;
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
