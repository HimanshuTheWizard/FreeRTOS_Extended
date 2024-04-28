/*
 * task_handler.c
 *
 *  Created on: Dec 25, 2022
 *      Author: Himanshu Mishra
 */
#include "task_handler.h"

/*===========Global variables definition===========*/
const char *Invalid_msg = "/=======INVALID ENTRY=======/\n";

/*========================================
 * Return Value : None
 * Parameters : pointer to parameter
 * Description : The task is invoked for handling the commands for appliances control
 * ========================================*/
void Appliances_Handling_Task(void *param)
{
	command_t cmd;
	BaseType_t status;
	const char *appliances_menu_msg =  "===========================\n"
									   "|     Appliances Menu     |\n"
									   "===========================\n"
									   "Bulb 1                --->0\n"
									   "Bulb 2                --->1\n"
									   "Fan                   --->2\n"
									   "Enter your choice: ";
	while(1)
	{
		/*Send the appliance control menu into the print queue*/
		status = xQueueSend(Print_queue, (void *)&appliances_menu_msg , portMAX_DELAY);
		if(pdTRUE == status)
		{
			/*Waiting for notification from menu task*/
			status = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
			if(pdTRUE == status)
			{
				/*Extracting command from command queue and placing in command structure*/
				Extract_command(&cmd);
				if(1u == cmd.len)
				{
					if(cmd.cmd_payload[0] == '0')
					{
						HAL_GPIO_TogglePin(GPIOD, LD3_Pin);
					}
					else if(cmd.cmd_payload[0] == '1')
					{
						HAL_GPIO_TogglePin(GPIOD, LD4_Pin);
					}
					else if(cmd.cmd_payload[0] == '2')
					{
						HAL_GPIO_TogglePin(GPIOD, LD5_Pin);
					}
					else
					{
						/*Sending "invalid message" warning to print queue*/
						status = xQueueSend(Print_queue, &Invalid_msg , portMAX_DELAY);
						if(pdPASS != status)
						{

						}
					}
				}
				else
				{
					/*Sending "invalid message" warning to print queue*/
					status = xQueueSend(Print_queue, &Invalid_msg , portMAX_DELAY);
					if(pdPASS != status)
					{

					}
				}
			}
		}
		/*Updating the current state of task*/
		Curr_state = e_main_menu_state;
		/*sending notification to menu task to display menu again*/
		xTaskNotify(Menu_display_task_handle, 0u, eNoAction);
	}
}
/*========================================
 * Return Value : None
 * Parameters :	Command structure pointer
 * Description : This function picks bytes from input queue and then place it in command structure
 * ========================================*/
void Extract_command(command_t *cmd)
{
	uint8_t item;
	uint8_t i = 0;
	BaseType_t status;
	/*Check if input data queue is not empty*/
	if(0u == uxQueueMessagesWaiting(Command_queue))
	{
		return;
	}
	do
	{
		status = xQueueReceive(Command_queue, (void *)&item, 0);
		if(pdTRUE  == status)
		{
			cmd->cmd_payload[i++] = item;
		}
	}
	while(item!='\n');
	cmd->cmd_payload[i-1] = '\0';
	cmd->len = i-1;
}

/*========================================
 * Return Value : None
 * Parameters : it takes void pointer as parameter which can be passed while creating the task
 * Description : The task picks command from queue, populates it in command structure and notify the task based on command and state
 * ========================================*/
void Command_Handling_Task(void *param)
{
	BaseType_t status;
	command_t cmd;
	while(1)
	{
		/*The task remains in blocked state until it receives notification from UART receive call back function*/
		status = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		if(pdPASS == status)
		{
			Extract_command(&cmd);
			if(Curr_state == e_main_menu_state)
			{
				/*Notifying the menu task by sending address of command structure object*/
				status = xTaskNotify(Menu_display_task_handle, (uint32_t)&cmd, eNoAction);
				if(pdPASS != status)
				{
					//Do something
				}
			}
			else if(Curr_state == e_appliances_control_state)
			{
				/*Notifying the appliances task by sending address of command structure object*/
				status = xTaskNotify(Appliances_task_handle, (uint32_t)&cmd, eNoAction);
				if(pdPASS != status)
				{
					//Do something
				}
			}
			else
			{
				//Do something
			}
		}
	}
}

/*========================================
 * Return Value : None
 * Parameters : it takes void pointer as parameter which can be passed while creating the task
 * Description : The task is responsible for menu display, taking user cmd and notifying other tasks.
 * ========================================*/
void Menu_Display_Handling_Task(void *param)
{
	BaseType_t status;
	uint32_t cmd_add;
	command_t *cmd = NULL;
	const char *menu_msg = "===========================\n"
						   "|           Menu          |\n"
						   "===========================\n"
						   "Appliances Control    --->0\n"
			               "Exit                  --->1\n"
						   "Enter your choice: ";
	while(1)
	{
		/*The message to be printed on command prompt, shall be sent to queue*/
		status = xQueueSend(Print_queue, (void *)&menu_msg , portMAX_DELAY);

		/*the task shall go to blocked state, waiting for notification from command handling task*/
		if(pdTRUE == status)
		{
			status = xTaskNotifyWait(0, 0, &cmd_add, portMAX_DELAY);
			if(pdTRUE == status)
			{
				cmd = (command_t *)cmd_add;
			}
		}
		if(cmd->len == 1)
		{
			/*User option: Appliances control*/
			if(cmd->cmd_payload[0] == '0')
			{
				Curr_state = e_appliances_control_state;
				status = xTaskNotify(Appliances_task_handle, 0u, eNoAction);/*Notify appliances handling task*/
				if(pdPASS == status)
				{
					status = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);/*Wait for next notification*/
					if(pdPASS != status)
					{
						//Do something
					}
				}
			}
			/*User option: Exit*/
			else if(cmd->cmd_payload[1] == '1')
			{
				//Update the state if needed in future implementation
				//Need to implement this part
				status = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
				if(pdPASS != status)
				{
					//Do something
				}
			}
			else
			{
				/*Sending "invalid message" warning to print queue*/
				status = xQueueSend(Print_queue, (void *)&Invalid_msg , portMAX_DELAY);
				if(pdTRUE != status)
				{
					//Do something
				}
			}
		}
		else
		{
			/*Sending "invalid message" warning to print queue*/
			status = xQueueSend(Print_queue, (void *)&Invalid_msg , portMAX_DELAY);
			if(pdTRUE != status)
			{
				//Do something
			}
		}
	}
}

/*========================================
 * Return Value : None
 * Parameters : it takes void pointer as parameter which can be passed while creating the task
 * Description : This function picks bytes from queue and send over UART
 * ========================================*/
void Print_Handling_Task(void *param)
{
	uint8_t *msg = NULL;
	BaseType_t queue_status;
	HAL_StatusTypeDef UART_status;
	while(1)
	{
		/*The task shall go to blocked state until print queue is populated*/
		/*Once data is received in print queue it start picking data from the queue*/
		queue_status = xQueueReceive(Print_queue, (void *)msg, portMAX_DELAY);
		if(pdPASS == queue_status)
		{
			/*Data is transmitted to serial terminal over UART*/
			UART_status = HAL_UART_Transmit(&huart2, msg, strlen((char *)msg), HAL_MAX_DELAY);
			if(UART_status != HAL_OK)
			{
				//Do something
			}
		}
	}
}


