/*
 * logger_file.h
 *
 *  Created on: Nov 12, 2020
 *      Author: Yolo
 */

#ifndef MAIN_INTERFACE_LOGGER_FILE_LOGGER_FILE_H_
#define MAIN_INTERFACE_LOGGER_FILE_LOGGER_FILE_H_

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
void logger_file_init( void );

bool logger_read_number_line( char *file_name );

bool logger_file_delete(char *file_name);

void logger_LoadInfo_WifiStation(bool smart_config);

#endif /* MAIN_INTERFACE_LOGGER_FILE_LOGGER_FILE_H_ */
