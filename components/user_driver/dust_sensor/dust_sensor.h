/*
 * stm32_com.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_DRIVER_DUST_SENSOR_H_
#define MAIN_USER_DRIVER_DUST_SENSOR_H_



/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "../../Common.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
	E_ESP_NONE,
	E_ESP_GET_STATE,
	E_ESP_SET_STATE,
	E_STM32_RESPONSE,
	E_STM32_TIMEOUT
}e_esp32stm32_com;

typedef enum
{
	E_COMMAND_GET_STATE = 0x01,
	E_COMMAND_SET_STATE = 0x02,
	E_COMMAND_RESPONSE = 0x03,
}e_anywave_command;

/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
bool anywave_parser_message(uint8_t *message, uint16_t length);

void anywave_tx_process_task_creat(void);

void anywave_set_event(e_esp32stm32_com event);


#endif /* MAIN_USER_DRIVER_DUST_SENSOR_H_ */
