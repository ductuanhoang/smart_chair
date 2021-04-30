/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "../Common.h"
// #include "../task/led_task.h"
#include "../task/bluetooth.h"
#include "../task/uart_task.h"
// #include "../task/oled_task.h"
#include "../task/wifi_task.h"
#include "../task/mqtt_task.h"
#include "../task/plan_task.h"

#include "../components/logger_file/logger_file.h"
#include "../components/peripheral/user_spi.h"
#include "../components/peripheral/user_adc.h"
#include "../components/json_parser/json_parser.h"

#include "../user_driver/stm32_com/stm32_com.h"
#include "../user_driver/oled_driver/oled_display.h"
/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

void GPIO_Init(void)
{
  gpio_config_t io_conf;

  //interrupt of rising edge
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  //bit mask of the pins, use GPIO0 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  io_conf.pull_up_en = 1;
  esp_err_t error = gpio_config(&io_conf); //configure GPIO with the given settings

  if (error != ESP_OK)
  {
    APP_LOGE("error configuring inputs\n");
  }
}

void app_main(void)
{
  APP_LOGI("--- APP_MAIN: Smart Anywave Update 09/03/2019......");
  APP_LOGI("--- APP_MAIN: Free memory: %d bytes", esp_get_free_heap_size());
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  GPIO_Init();

  logger_file_init();
  logger_LoadInfo_WifiStation(false);

  uart_com2stm32_init();

  
  uart_com2stm32_start();
  plan_task();
}
