/*
 * mqtt_task.c
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
#include "mqtt_task.h"
#include "../../components/json_parser/json_parser.h"
#include "../../Common.h"
#include "../../main.h"

#include "esp_wifi.h"
#include "esp_system.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

#include "esp_http_client.h"

/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
static const char *TAG = "MQTTS_EXAMPLE";
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
const char *client_cert_pem_start = "-----BEGIN CERTIFICATE-----\n"
                                    "MIIEgDCCAmgCCQCNHYxckGcHjzANBgkqhkiG9w0BAQsFADBuMQswCQYDVQQGEwJW\n"
                                    "TjEPMA0GA1UECAwGSGEgTm9pMQ8wDQYDVQQHDAZIYSBOb2kxEDAOBgNVBAoMB0Vs\n"
                                    "aWZldXAxFTATBgNVBAsMDElvVCBQbGF0Zm9ybTEUMBIGA1UEAwwLZWxpZmV1cC5j\n"
                                    "b20wIBcNMjEwNTE2MTAyNjE4WhgPMjA3MTA1MDQxMDI2MThaMIGTMQswCQYDVQQG\n"
                                    "EwJWTjEPMA0GA1UECAwGSGEgTm9pMQ8wDQYDVQQHDAZIYSBOb2kxEDAOBgNVBAoM\n"
                                    "B0VsaWZldXAxDDAKBgNVBAsMA0lvVDEiMCAGA1UEAwwZU21hcnRTdG9wIElvVCBD\n"
                                    "ZXJ0aWZpY2F0ZTEeMBwGCSqGSIb3DQEJARYPaHV5QGVsaWZldXAuY29tMIIBIjAN\n"
                                    "BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqq2uEgH2hhNm1ZmUbAgIYFWvAduj\n"
                                    "UbIkOPz7fY2QynxMAyWclB6c/Nz2ZBudKCbkUYaKHwzrk0rHwbF8WQiom2JHzsZx\n"
                                    "QA4yJDuJ7aV9Zw+9btCnmwYSlWQ80TdfUcZgCedXusXvoPo1ajVtnGOlhsceY/0w\n"
                                    "0mT0OJ9YKuOi0yaPrJas6bvw5xlVwhFN5r1PgFZ/mEZZLVLI/ki96Z/7pqKSD5g7\n"
                                    "bz/nOPOrq5/LqMUXt8qjZv7BeYm+F+7ml4mu6WCVIE9U2k6o8ci/pjU5KQaAaqcL\n"
                                    "5ivvCayQzG+5R8lhU96ppyoYojEy9F+3gp9Qd0hoXLt+uTbnNpXtY2dbvQIDAQAB\n"
                                    "MA0GCSqGSIb3DQEBCwUAA4ICAQBjZMpSk0MgdGSJD0IWc9bv7do8ZaoMv8tETCQK\n"
                                    "MIKNM8XBGpuYDQyS48dUUImmPpd+yBNDuqoBQs6xp9phDrwBXpSPr/vTiAAVWG51\n"
                                    "zNRTSVhGgNGr/dd6WXWQApF7h8hAv2J9CL4q55eMPkSHyD9G7DuDbhL2ilP6PjE5\n"
                                    "BBiXpMmvbN6kq+2PDS/TZwAHeRc1IFpbyllgzuOEqkRXnllMVPRWDcH4kp7Nr3il\n"
                                    "jWI6zGbezFn23H+FNQatpW9EpjnOmdyfsh5OENf1nZ4zAHkO4MHoI3nWht6zWTMR\n"
                                    "E2nTM6zi0aUU+aZO9MwWkdQkx8dpJl6oZ2Bcjx4mizDHoDDJN3fAT20IdvkcBwAb\n"
                                    "L6ypDECy34LPq7w0jqeNuPCVXPz/VDtT5Dj40G4ChNVzmgapLU4LJZJAetdCJqja\n"
                                    "hUBZ3/KHp0FcrIKEJo39HxZO2uDht0pK4OXTnzvFzsaAfMhtwCMaK4qWYmJqLxVl\n"
                                    "Br5iWxWGPW2n/nhLNn8cU3Kl3b1wlwwJD65uQrRTxXn5Ux1BGaXi/w/8tqX8pbJS\n"
                                    "WqaU6SEtS8xq9PIFqi+0BPrJN062aQ+KqWxkOpFSvg8oRc0H31K4LKy+YkgGzjYs\n"
                                    "Pk89gojkYIOritKubZeLDOzjJKvYAEChoUf/aNWKt5m/mJZaCkh78bo5F8kvE3Az\n"
                                    "8vmrjA==\n"
                                    "-----END CERTIFICATE-----\n";

const char *client_key_pem_start = "-----BEGIN RSA PRIVATE KEY-----\n"
                                   "MIIEpAIBAAKCAQEAqq2uEgH2hhNm1ZmUbAgIYFWvAdujUbIkOPz7fY2QynxMAyWc\n"
                                   "lB6c/Nz2ZBudKCbkUYaKHwzrk0rHwbF8WQiom2JHzsZxQA4yJDuJ7aV9Zw+9btCn\n"
                                   "mwYSlWQ80TdfUcZgCedXusXvoPo1ajVtnGOlhsceY/0w0mT0OJ9YKuOi0yaPrJas\n"
                                   "6bvw5xlVwhFN5r1PgFZ/mEZZLVLI/ki96Z/7pqKSD5g7bz/nOPOrq5/LqMUXt8qj\n"
                                   "Zv7BeYm+F+7ml4mu6WCVIE9U2k6o8ci/pjU5KQaAaqcL5ivvCayQzG+5R8lhU96p\n"
                                   "pyoYojEy9F+3gp9Qd0hoXLt+uTbnNpXtY2dbvQIDAQABAoIBAC92pw6o7xZv9Mv3\n"
                                   "rpewUCwCB+37V1qTsJEMgR90K8yzbiv93KIwNTX4eKh0KWsODbZCNMzXufc293/8\n"
                                   "zHix+LllRlTRSJMon3cF+6BTwiDT9rkHW2S39pkGzAbeYCqMgQ6f//yXqMDac9o6\n"
                                   "S2YPK+vkGaZytY38txG79jfPH+uZvFHnraVbulKWF3ib/nG8DnOFPeCbkNYwk2CT\n"
                                   "RlcGjlrBGMXTPz60OPykUhHqAggvRa1EgOEJjHPW7qwhApe1L5nmBf/9LVG/7YHR\n"
                                   "442ahMQnjazVmdM3BiQXzxeoqk8V0UCswP9ur+QNKPFkw9q/FLRMu9LfyGtw6IhU\n"
                                   "HiU47sECgYEA1ybctYJfkfSdmdiI8YoGsINFrAasSFM1SWo1ONrtq+ZTgKnvd3qB\n"
                                   "q0fD97YpYESgBNOSci7C3uNPniyBrATYlQDgMqEm4njxbcsUlmyreufDKU22R8J/\n"
                                   "KN1uL0vaeZZnfmEvgRUjcr3JNnJe2p8OgxyEgzqBm5IkMAAGRjOKH+0CgYEAyxVB\n"
                                   "SLSHHon4dA3HzwdYS5XlwPy3G6hFnyOcFRZhqhhG0K9yKklW5bnLzqwYi+umAoUC\n"
                                   "Q+FLE6HaLc+XN9Z/f4jSMZ0+UCZSm7KBH9Wt80HrMzRTA12pWxRhDzXi6cJSIq9J\n"
                                   "0QsOZbBOdrSwECbn+CKg8f3rzER7bwnFdYb+kRECgYEAqz1aEvkmGaPov+bw79Wc\n"
                                   "h2aj0EwrWREo6zqmC49r9RJHybL3Tk/p3qoq1gBdJCradZzzBQAkx4OB/fGMb54X\n"
                                   "x1hAGOvcaAo8ldc5lpP9U8Acu8YHW0v5K0w6A1jLFVTZIGQ3i/SIFy3odPZIepZ3\n"
                                   "1XCgI1Ywi+Kf/Lg4Ri2FNO0CgYEAmMmC9korpgQzUkzT2KQz/5nk4w6+TCaLSrEl\n"
                                   "yo+uJqRhErwMblgC8o6YEQNU7F/748Vh8OPc8gZA+VpG8JGGFtM/IGim6vIKEG15\n"
                                   "zBOc7XjYlQt2sP+UXJu2chUehLPXy5SJOqbQzByay6AhHeXHe93BrI5XCrUzEFUP\n"
                                   "o95OQ6ECgYBz9AIaPjG46EkbMi4tivA2WOFDIQ20EZ4Mx8qhAeF7haKK8JDFuVZo\n"
                                   "pttkbbTedsAN+kBCZ88c9uvf4PqFNqPN0pdcB4a6lqlx8GK9xQl5zpT+71dKG5r1\n"
                                   "Xor3UGRUO5cB1V3O+2JAjeS3AZHCdr720XoVRKoQYzFulzZbJQ2QzA==\n"
                                   "-----END RSA PRIVATE KEY-----\n";

const char *server_cert_pem_start = "-----BEGIN CERTIFICATE-----\n"
                                    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
                                    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
                                    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
                                    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
                                    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
                                    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
                                    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
                                    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
                                    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
                                    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
                                    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
                                    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
                                    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
                                    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
                                    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
                                    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
                                    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
                                    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
                                    "-----END CERTIFICATE-----\n";

static esp_mqtt_client_handle_t client;

static void mqtt_app_start(void);
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
static void mqtt_send_task(void *pvParameters);
// static void mqtt_get_cer(void);

// const char *topic_sub = "job/ard-smartstop/myhome/ard-smartstop-123456/list";
// const char *topic_pub = "job/ard-smartstop/myhome/ard-smartstop-123456/status";

// Telemetry	<environment>/dt/ard-smartstop/<client_id>
// 	        <environment>/dt/ard-smartstop/<client_id>/error
// Job	        <environment>/job/ard-smartstop/<client_id>/accepted
// 	        <environment>/job/ard-smartstop/<client_id>/status

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
static void error_message_send(void);
static char *wifi_get_mac(void);
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void mqtt_task_start(void)
{
    mqtt_app_start(); // init mqtt connect to AWS
    xTaskCreatePinnedToCore(mqtt_send_task, "mqtt_send_task", 2 * 1024, NULL, 6 | portPRIVILEGE_BIT, NULL, 1);
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/

static void mqtt_app_start(void)
{
    // mqtt_get_cer();
    char client_id[50] = "ard-smartstop-";
    strcat(client_id, wifi_get_mac());
    APP_LOGI("Client id: %s", client_id);
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://a3qnicqxfi1gf7-ats.iot.ap-southeast-1.amazonaws.com:8883",
        .event_handle = mqtt_event_handler,
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
        .cert_pem = (const char *)server_cert_pem_start,
        .client_id = client_id,
        // .use_global_ca_store = true,
    };

    //using cloud mqt
    // const esp_mqtt_client_config_t mqtt_cfg = {
    //     .host = "m13.cloudmqtt.com",
    //     .port = 11734,
    //     .client_id = "1234adc",
    //     .username = "wcewiofp",
    //     .password = "fyFZMCLNvoD9",
    //     .keepalive = 60,
    //     .event_handle = mqtt_event_handler,
    // };
    // memset(mqtt_cfg.client_id, 0x00, sizeof(MQTT_MAX_CLIENT_LEN));
    memset(mqtt_config.mqtt_topic_pub, 0x00, sizeof(mqtt_config.mqtt_topic_pub));
    memset(mqtt_config.mqtt_topic_pub_err, 0x00, sizeof(mqtt_config.mqtt_topic_pub_err));
    memset(mqtt_config.mqtt_topic_jobsub, 0x00, sizeof(mqtt_config.mqtt_topic_jobsub));
    memset(mqtt_config.mqtt_topic_jobpub, 0x00, sizeof(mqtt_config.mqtt_topic_jobpub));

    sprintf(mqtt_config.mqtt_topic_pub, "stag/dt/ard-smartstop/%s", client_id);
    sprintf(mqtt_config.mqtt_topic_pub_err, "stag/dt/ard-smartstop/%s/error", client_id);
    sprintf(mqtt_config.mqtt_topic_jobsub, "stag/job/ard-smartstop/%s/accepted", client_id);
    sprintf(mqtt_config.mqtt_topic_jobpub, "stag/job/ard-smartstop/%s/status", client_id);

    // sprintf((char *)mqtt_cfg.client_id, "ard-smartstop-%s", wifi_get_mac());
    APP_LOGI("mqtt_topic_pub = %s", mqtt_config.mqtt_topic_pub);
    APP_LOGI("mqtt_topic_pub_err = %s", mqtt_config.mqtt_topic_pub_err);
    APP_LOGI("mqtt_topic_jobsub = %s", mqtt_config.mqtt_topic_jobsub);
    APP_LOGI("mqtt_topic_jobpub = %s", mqtt_config.mqtt_topic_jobpub);
    APP_LOGI("mqtt_cfg.client_id = %s", mqtt_cfg.client_id);

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        deive_data.mqtt_status = true;
        msg_id = esp_mqtt_client_subscribe(client, mqtt_config.mqtt_topic_jobsub, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        deive_data.mqtt_status = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, topic_pub, "data", 0, 0, 0);
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
        bool status = json_parser_job((const char *)event->data, event->data_len);
        APP_LOGI("status = %d", status);
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

/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
uint32_t previous_time_start = 0;
uint8_t state_send_data = 0;
bool time_inter_val_send = false;
static void mqtt_send_task(void *pvParameters)
{
    int msg_id;
    while (1)
    {
        if (deive_data.mqtt_status == true)
        {
            if (state_send_data == 0)
            {
                previous_time_start = usertimer_gettick();
                state_send_data = 1;
            }
            else if (state_send_data == 1)
            {
                if ((usertimer_gettick() - previous_time_start) > (mqtt_config.time_interval_send_data * 1000))
                {
                    state_send_data = 0;
                    time_inter_val_send = true;
                    char *message_packet = (char *)malloc(200 * sizeof(char));
                    memset(message_packet, 0x00, 200 * sizeof(char));
                    json_packet_message_sensor(message_packet);
                    APP_LOGI("send : = %s", message_packet);
                    msg_id = esp_mqtt_client_publish(client, mqtt_config.mqtt_topic_pub, message_packet, 0, 0, 0);
                    memset(message_packet, 0x00, 200 * sizeof(char));
                    free(message_packet);
                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                }
            }

            if (time_inter_val_send)
            {
                error_message_send();
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
uint32_t previous_time_error = 0;
uint8_t state_send_error = 0;
static void error_message_send(void)
{
    int msg_id;
    switch (state_send_error)
    {
    case 0:
        previous_time_error = usertimer_gettick();
        state_send_error = 1;
        break;
    case 1:
        if (deive_data.sensor.dust_error)
        {
            if (usertimer_gettick() - previous_time_error > 5000)
            {
                char *message_packet = (char *)malloc(200 * sizeof(char));
                memset(message_packet, 0x00, 200 * sizeof(char));
                json_packet_message_error(message_packet, 1);
                msg_id = esp_mqtt_client_publish(client, mqtt_config.mqtt_topic_pub_err, message_packet, 0, 0, 0);
                memset(message_packet, 0x00, 200 * sizeof(char));
                free(message_packet);

                previous_time_error = usertimer_gettick();
                state_send_error = 2;
            }
        }
        else
            state_send_error = 2;
        break;
    case 2:
        if (deive_data.sensor.sht3x_error)
        {
            if (usertimer_gettick() - previous_time_error > 5000)
            {
                char *message_packet = (char *)malloc(200 * sizeof(char));
                memset(message_packet, 0x00, 200 * sizeof(char));
                json_packet_message_error(message_packet, 2);
                msg_id = esp_mqtt_client_publish(client, mqtt_config.mqtt_topic_pub_err, message_packet, 0, 0, 0);
                memset(message_packet, 0x00, 200 * sizeof(char));
                free(message_packet);

                previous_time_error = usertimer_gettick();
                state_send_error = 3;
            }
        }
        else
            state_send_error = 3;
        break;

    case 3:
        if (deive_data.sensor.rain_error)
        {
            if (usertimer_gettick() - previous_time_error > 5000)
            {
                char *message_packet = (char *)malloc(200 * sizeof(char));
                memset(message_packet, 0x00, 200 * sizeof(char));
                json_packet_message_error(message_packet, 3);
                msg_id = esp_mqtt_client_publish(client, mqtt_config.mqtt_topic_pub_err, message_packet, 0, 0, 0);
                memset(message_packet, 0x00, 200 * sizeof(char));
                free(message_packet);

                previous_time_error = usertimer_gettick();
                state_send_error = 0;
            }
        }
        else
        {
            state_send_error = 0;
            time_inter_val_send = false;
        }
        break;
    default:
        break;
    }
}

static char *wifi_get_mac(void)
{
    char *mac_add;
    mac_add = (char *)malloc(18 + 1);
    //Get the derived MAC address for each network interface
    uint8_t derived_mac_addr[6] = {0};
    //Get MAC address for WiFi Station interface
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_STA));
    sprintf(mac_add, "%x:%x:%x:%x:%x:%x", derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2], derived_mac_addr[3],
            derived_mac_addr[4], derived_mac_addr[5]);
    APP_LOGD("wifi_get_mac end = %s", mac_add);
    return mac_add;
}

void mqtt_get_cer(void)
{
    APP_LOGI("-------------mqtt_get_cer");
    esp_http_client_config_t config = {
        .url = "https://iot.smartstop.elifeup.com/iot-thing/active",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0}; // Buffer to store response of http request
    // POST Request
    const char *post_data = "{\"thing_id\":\"ard-smartstop-123456\"}";
    esp_http_client_set_url(client, "https://iot.smartstop.elifeup.com/iot-thing/active");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", "Bearer Y0VfmU0GgLm3n0e9ZdelERL8M5QOQVHu");
    esp_http_client_set_header(client, "X-SmartStop-Api-Key", "NW4LUY150TJBIFYKGYEZ");
    esp_err_t err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    else
    {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0)
        {
            ESP_LOGE(TAG, "Write failed");
        }
        int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
        if (data_read >= 0)
        {
            APP_LOGI("HTTP GET Status = %d, content_length = %d",
                     esp_http_client_get_status_code(client),
                     esp_http_client_get_content_length(client));
            ESP_LOG_BUFFER_HEX(TAG, output_buffer, strlen(output_buffer));
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read response");
        }
    }
}