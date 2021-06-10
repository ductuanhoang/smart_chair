/*
 * sht30_sensor.c
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
#include "sht30_sensor.h"
#include "../sht3x/sht3x.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/

#define I2C_BUS 0
#define I2C_SCL_PIN 14
#define I2C_SDA_PIN 13
#define I2C_FREQ I2C_FREQ_100K

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static sht3x_sensor_t *sensor; // sensor device data structure
static void sht30_task(void *pvParameters);
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
void sht30_task_start(void)
{
    xTaskCreatePinnedToCore(sht30_task, "sht30_task", 2 * 1024, NULL, 5 | portPRIVILEGE_BIT, NULL, 1);
}
/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
/*
 * User task that fetches latest measurement results of sensor every 2
 * seconds. It starts the SHT3x in periodic mode with 1 measurements per
 * second (*sht3x_periodic_1mps*).
 */
static void sht30_task(void *pvParameters)
{
    float temperature;
    float humidity;
    // Start periodic measurements with 1 measurement per second.
    sht3x_start_measurement(sensor, sht3x_periodic_1mps, sht3x_high);

    // Wait until first measurement is ready (constant time of at least 30 ms
    // or the duration returned from *sht3x_get_measurement_duration*).
    vTaskDelay(sht3x_get_measurement_duration(sht3x_high));

    TickType_t last_wakeup = xTaskGetTickCount();

    while (1)
    {
        // Get the values and do something with them.
        if (sht3x_get_results(sensor, &temperature, &humidity))
        {
            printf("%.3f SHT3x Sensor: %.2f °C, %.2f %%\n",
                   (double)sdk_system_get_time() * 1e-3, temperature, humidity);
            deive_data.sensor.temperature = temperature;
            deive_data.sensor.humidiy = humidity;
            deive_data.sensor.sht3x_error = false;
        }
        else
        {
            APP_LOGD("read sensor fail");
            deive_data.sensor.sht3x_error = true;
            deive_data.sensor.temperature = 0;
            deive_data.sensor.humidiy = 0;
        }

        // Wait until 2 seconds (cycle time) are over.
        vTaskDelayUntil(&last_wakeup, 2000 / portTICK_PERIOD_MS);
    }
}

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void sht3x_init(void)
{
    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);

    sensor = sht3x_init_sensor(I2C_BUS, SHT3x_ADDR_1);
}

/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
