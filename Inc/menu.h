/*
 * menu.h
 *
 *  Created on: Dec 1, 2024
 *      Author: wataoxp
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "main.h"

typedef enum{
	TuneUp,
	TuneDown,
}SelectMode;

void SerialHome(UART_HandleTypeDef *huart,I2C_HandleTypeDef *hi2c,char *key);
void CommandManual(UART_HandleTypeDef *huart);

#endif /* INC_MENU_H_ */
