/*
 * ota_task.c
 *
 *  Created on: Jan 7, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "../../Common.h"
#include "ota_task.h"
#include "../../components/json_parser/json_parser.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"

/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void ota_start(void *pvParameters);
static bool ota_get_status(void);
static bool ota_process(void);
void wifi_get_mac_02(char *mac_add);

uint8_t request_message_frist_time = 0;
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
uint32_t previous_time_ota_start = 0;
uint8_t state_ota = 0;

char ota_infor_message[800] = {0};
uint16_t ota_infor_length = 0;
bool ota_call_first_time = true;
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
esp_err_t _ota_http_event_handle(esp_http_client_event_t *evt)
{
	switch (evt->event_id)
	{
	case HTTP_EVENT_ERROR:
		APP_LOGI("HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		APP_LOGI("HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		APP_LOGI("HTTP_EVENT_HEADER_SENT");

		break;
	case HTTP_EVENT_ON_HEADER:
		APP_LOGI("HTTP_EVENT_ON_HEADER");

		// length = sprintf(cer_link + length, "%.*s", evt->data_len, (char *)evt->data);
		break;
	case HTTP_EVENT_ON_DATA:
		APP_LOGI("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		if (!esp_http_client_is_chunked_response(evt->client))
		{
			if (request_message_frist_time >= 1)
			{
				request_message_frist_time = 2;
				ota_infor_length += sprintf(ota_infor_message + ota_infor_length, "%.*s", evt->data_len, (char *)evt->data);
			}
			else
				request_message_frist_time = 1;
			printf("%.*s", evt->data_len, (char *)evt->data);
		}
		break;
	case HTTP_EVENT_ON_FINISH:
		APP_LOGI("HTTP_EVENT_ON_FINISH");
		printf("%.*s", evt->data_len, (char *)evt->data);
		break;
	case HTTP_EVENT_DISCONNECTED:
		APP_LOGI("HTTP_EVENT_DISCONNECTED ");
		break;
	}
	return ESP_OK;
}
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void ota_task(void)
{
	xTaskCreatePinnedToCore(ota_start, "ota_task", 6 * 1024, NULL, 5 | portPRIVILEGE_BIT, NULL, 0);
}

/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
static void ota_start(void *pvParameters)
{
	bool status = false;

	while (1)
	{
		if ((deive_data.wifi_status == true))
		{
			if (state_ota == 0)
			{
				APP_LOGW("ota_task_call 1");
				previous_time_ota_start = usertimer_gettick();
				state_ota = 1;
			}
			else if (state_ota == 1)
			{
				APP_LOGW("ota_task_call 2");
				if (ota_call_first_time)
				{
					ota_call_first_time = false;
					if ((usertimer_gettick() - previous_time_ota_start) > (20 * 1000)) // cal first time after 20s
					{
						state_ota = 0;
						status = ota_get_status();
						if (status == true)
							ota_process();
						state_ota = 0;
					}
					ota_call_first_time = false;
				}
				else
				{
					if ((usertimer_gettick() - previous_time_ota_start) > (mqtt_config.time_interval_check_ota * 1000))
					{
						state_ota = 0;
						status = ota_get_status();
						if (status == true)
							ota_process();
						state_ota = 0;
					}
				}
			}
		}
		APP_LOGW("ota_task_call = %d", state_ota);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

static bool ota_get_status(void)
{
	bool ota_get = false;
	request_message_frist_time = 0;
	esp_http_client_handle_t client_2;
	APP_LOGI("-------------ota_get_status");
	esp_http_client_config_t config = {
		.url = "https://iot.smartstop.elifeup.com/iot-thing/ota",
		.event_handler = _ota_http_event_handle,
		.timeout_ms = 50000,
	};
	client_2 = esp_http_client_init(&config);
	// POST Request
	const char *post_data;
	post_data = (char *)malloc(200 + 1);
	char sha256_buf[65];
	esp_ota_get_app_elf_sha256(sha256_buf, sizeof(sha256_buf));
	char *mac_add = (char *)malloc(20 * sizeof(char));
	memset(mac_add, 0x00, 20 * sizeof(char));
	wifi_get_mac_02(mac_add);
	sprintf(post_data, "{\"thing_id\":\"ard-smartstop-%s\",\"firmware_hash\":\"%s\"}", mac_add, sha256_buf);
	esp_http_client_set_method(client_2, HTTP_METHOD_POST);
	esp_http_client_set_header(client_2, "Content-Type", "application/json");
	esp_http_client_set_header(client_2, "Authorization", "Bearer Y0VfmU0GgLm3n0e9ZdelERL8M5QOQVHu");
	esp_http_client_set_header(client_2, "X-SmartStop-Api-Key", "NW4LUY150TJBIFYKGYEZ");

	APP_LOGI("post_data = %s", post_data);
	esp_err_t err = esp_http_client_perform(client_2);
	err = esp_http_client_open(client_2, strlen(post_data));
	char output_buffer[1000];
	if (err != ESP_OK)
	{
		APP_LOGE("Failed to open HTTP connection: %s", esp_err_to_name(err));
	}
	else
	{
		int wlen = esp_http_client_write(client_2, post_data, strlen(post_data));
		if (wlen < 0)
		{
			APP_LOGI("Write failed");
		}
		int data_read = esp_http_client_read_response(client_2, output_buffer, 1000);
	}

	if (request_message_frist_time == 2)
	{
		APP_LOGI("request_message_frist_time = %d", ota_infor_length);
		ota_get = json_parser_ota_link(ota_infor_message, ota_infor_length);
	}
	else
		ota_get = false;

	esp_http_client_cleanup(client_2);
	free(post_data);
	free(mac_add);
	APP_LOGW("ota_get = %d", ota_get);
	if (ota_get == false)
	{
		ota_infor_length = 0;
		memset(ota_message_link, 0x00, sizeof(ota_message_link));
	}
	return ota_get;
}

static bool ota_process(void)
{
	bool ota_process_status = false;
	APP_LOGI("Starting Advanced OTA");

	esp_err_t ota_finish_err = ESP_OK;
	esp_http_client_config_t config = {
		.url = ota_message_link,
		// .url = "https://iot-storages.s3.ap-southeast-1.amazonaws.com/firmware/smartstop_v2.bin?X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIA2KOWOZWZDNTAATOB%2F20210529%2Fap-southeast-1%2Fs3%2Faws4_request&X-Amz-Date=20210529T092856Z&X-Amz-SignedHeaders=host&X-Amz-Expires=3600&X-Amz-Signature=6f677f0c559a80c074e862bb860d922fc1588e7a0305b1153fba1e48effb5ebf",
		// .cert_pem = (char *)server_cert_pem_start,
		.timeout_ms = 20000,
	};
	// esp_http_client_set_url(client_2, ota_message_link);

	esp_https_ota_config_t ota_config = {
		.http_config = &config,
	};
	esp_https_ota_handle_t https_ota_handle = NULL;

	esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
	if (err != ESP_OK)
	{
		APP_LOGE("ESP HTTPS OTA Begin failed");
		ota_process_status = false;
	}

	esp_app_desc_t app_desc;
	err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
	if (err != ESP_OK)
	{
		APP_LOGE("esp_https_ota_read_img_desc failed");
		ota_process_status = false;
		goto ota_end;
	}

	while (1)
	{
		err = esp_https_ota_perform(https_ota_handle);
		if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
		{
			break;
		}
		// esp_https_ota_perform returns after every read operation which gives user the ability to
		// monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
		// data read so far.
		APP_LOGI("Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
	}

	if (esp_https_ota_is_complete_data_received(https_ota_handle) != true)
	{
		// the OTA image was not completely received and user can customise the response to this situation.
		APP_LOGE("Complete data was not received.");
		ota_process_status = false;
	}
ota_end:
	ota_finish_err = esp_https_ota_finish(https_ota_handle);
	if ((err == ESP_OK) && (ota_finish_err == ESP_OK))
	{
		APP_LOGI("ESP_HTTPS_OTA upgrade successful. Rebooting ...");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		esp_restart();
	}
	else
	{
		if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED)
		{
			APP_LOGE("Image validation failed, image is corrupted");
		}
		APP_LOGE("ESP_HTTPS_OTA upgrade failed %d", ota_finish_err);
		ota_process_status = false;
	}

	// free
	ota_infor_length = 0;
	memset(ota_message_link, 0x00, sizeof(ota_message_link));
	return ota_process_status;
}

void wifi_get_mac_02(char *mac_add)
{
	char mac_add_buff[18];
	memset(mac_add_buff, 0x00, sizeof(mac_add_buff));
	//Get the derived MAC address for each network interface
	uint8_t derived_mac_addr[6] = {0};
	//Get MAC address for WiFi Station interface
	ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_STA));
	sprintf(mac_add_buff, "%x:%x:%x:%x:%x:%x", derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2], derived_mac_addr[3],
			derived_mac_addr[4], derived_mac_addr[5]);
	strcpy(mac_add, mac_add_buff);
	APP_LOGD("wifi_get_mac end = %s", mac_add);
}

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
