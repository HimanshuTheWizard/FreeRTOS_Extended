/*
 * task_handler.h
 *
 *  Created on: Dec 25, 2022
 *      Author: Himanshu Mishra
 */

#ifndef INC_TASK_HANDLER_H_
#define INC_TASK_HANDLER_H_

/*===========header files inclusion===========*/
#include "main.h"
#include "string.h"

/*===========function prototypes===========*/
void Appliances_Handling_Task(void *param);
void Command_Handling_Task(void *param);
void Menu_Display_Handling_Task(void *param);
void Print_Handling_Task(void *param);
void Extract_command(command_t *cmd);

#endif /* INC_TASK_HANDLER_H_ */
