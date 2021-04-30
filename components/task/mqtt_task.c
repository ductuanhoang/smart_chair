/*
 * mqtt_task.c
 *
 *  Created on: Apr 25, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt_task.h"

#include "../json_parser/json_parser.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
#define MQTT_DATA_LENGTH_MAX			128
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/

static const char *TAG = "MQTT_EXAMPLE";


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
typedef struct
{
	esp_mqtt_client_config_t 	xClientInfo;
	char 						PublishTopic[64];
	char						SubcribeTopic[64];
}MQTTClient_Info;


MQTTClient_Info xMQTTClient_Info =
{
	.xClientInfo =
	{
		.host			= "m13.cloudmqtt.com",
		.port			= 11734,
		.client_id		= "1234adc",
		.username		= "wcewiofp",
		.password		= "fyFZMCLNvoD9",
		.keepalive		= 60,
		// .event_handle	= xMQTTClient_Event_Handler,
	},

	.PublishTopic	= "/pub_anywave/",
	.SubcribeTopic	= "/sub_anywave/",
};
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void mqtt_task_start(void)
{
	printf("\r\n--- MQTTClient: Mqtt client start...\r\n");
	memset(xMQTTClient_Info.PublishTopic, 0x00, sizeof(xMQTTClient_Info.PublishTopic));
	memset(xMQTTClient_Info.SubcribeTopic, 0x00, sizeof(xMQTTClient_Info.SubcribeTopic));
	sprintf(xMQTTClient_Info.PublishTopic, "/pub_anywave/%s", wifi_info.wifi_mac);
	sprintf(xMQTTClient_Info.SubcribeTopic, "/sub_anywave/%s", wifi_info.wifi_mac);

#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(xMQTTClient_Info.xClientInfo.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        xMQTTClient_Info.xClientInfo.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&xMQTTClient_Info.xClientInfo);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "\r\n--- MQTTClient: MQTT_EVENT_CONNECTED\r\n");
            ESP_LOGI(TAG, "--- MQTTClient: subcribe topic = %s\r\n", xMQTTClient_Info.SubcribeTopic);
            msg_id = esp_mqtt_client_subscribe(client, xMQTTClient_Info.SubcribeTopic, 0);
            ESP_LOGI(TAG,"--- MQTTClient: mqtt client sent subscribe at topic \"%s\", msg_id = %d\r\n", xMQTTClient_Info.SubcribeTopic, msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            if(event->data_len > MQTT_DATA_LENGTH_MAX)
            	break;
            char buffer_message[MQTT_DATA_LENGTH_MAX];
            sprintf(buffer_message, "%s", event->data);
            buffer_message[event->data_len + 1] = 0x00;
         	bool status = json_parser_message(buffer_message, 0);
			ESP_LOGI(TAG, "json_parser_message mqtt = %d", status);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}


/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/

