/*
 * rain_sensor.c
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
#include "rain_sensor.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
#define DEFAULT_VREF 1100       //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64        //Multisampling
#define TIME_READ_INTERVAL 1000 // 300ms
#define RAIN_THRESHOLD_DETECTED 2.5
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
static esp_adc_cal_characteristics_t *adc_chars;

static const adc_channel_t channel = ADC_CHANNEL_6; //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

static uint8_t adc_state_read = 0;
static uint32_t previous_time_adc_read = 0;

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void check_efuse(void);
static void print_char_val_type(esp_adc_cal_value_t val_type);
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
void rain_sensor_init(void)
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();
    //Configure ADC
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void rain_sensor_process(void)
{
    uint32_t adc_reading = 0;
    switch (adc_state_read)
    {
    case 0:
        previous_time_adc_read = usertimer_gettick();
        adc_state_read = 1;
        break;
    case 1:
        if (usertimer_gettick() - previous_time_adc_read > TIME_READ_INTERVAL)
        {
            //Multisampling
            for (int i = 0; i < NO_OF_SAMPLES; i++)
            {
                if (unit == ADC_UNIT_1)
                {
                    adc_reading += adc1_get_raw((adc1_channel_t)channel);
                }
                else
                {
                    int raw;
                    adc2_get_raw((adc2_channel_t)channel, width, &raw);
                    adc_reading += raw;
                }
            }
            adc_reading /= NO_OF_SAMPLES;
            //Convert adc_reading to voltage in mV
            uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
            printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
            if (voltage < RAIN_THRESHOLD_DETECTED)
                deive_data.sensor.rain_status = true;
            else
                deive_data.sensor.rain_status = false;

            adc_state_read = 0;
        }
        break;
    default:
        break;
    }
}

/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
static void check_efuse(void)
{
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        printf("eFuse Two Point: Supported\n");
    }
    else
    {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        printf("eFuse Vref: Supported\n");
    }
    else
    {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        printf("Characterized using Two Point Value\n");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        printf("Characterized using eFuse Vref\n");
    }
    else
    {
        printf("Characterized using Default Vref\n");
    }
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
