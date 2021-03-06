/*
 * GSM_cnf.h
 *
 *  Created on: 13 ���. 2017 �.
 *      Author: Dumitru
 */

#ifndef GSM_CNF_H_
#define GSM_CNF_H_


/***************************Imports*************************************************/

#include <stdint.h>

/***************************Public Macro Definitions********************************/

#define MY_NUMBER               "+37368047277"
#define SERVER_HOST             "217.26.174.207"
#define SERVER_CONNECTION_PORT  "351"

#define GSM_COMMAND_START_WATERING                                 0x01
#define GSM_COMMAND_COLLECTING_DATA                                0x02

/***************************Public Type Definitions********************************/



/***************************Public Data Definitions********************************/



/***************************Public Function Definitions****************************/

uint8_t GSM_IsCommand(void);
uint8_t GSM_ListenToServerCommands(void);

#endif /* GSM_CNF_H_ */
