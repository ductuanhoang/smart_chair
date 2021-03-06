/*
 * uart_task.h
 *
 *  Created on: Nov 24, 2020
 *      Author: Yolo
 */

#ifndef MAIN_TASK_UART_TASK_H_
#define MAIN_TASK_UART_TASK_H_
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


/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
void uart_dust_sensor_start(void);

void uart_dust_sensor_send(unsigned char* buf, unsigned char size);
void uart_dust_sensor_init(void);

#endif /* MAIN_TASK_UART_TASK_H_ */
