/*
 * menu.c
 *
 *  Created on: Dec 1, 2024
 *      Author: wataoxp
 */
#include "main.h"
#include "menu.h"
#include "Radio.h"
#include <stdio.h>
#include <string.h>

const uint16_t RadioStationList[8] ={
		FM_YOKOHAMA,		//Fm yokohama 84.7MHz
		FM_TOKYO,			//TOKYO FM 80.0MHz
		FM_CHIBA,			//BAYFM 78.0MHz
		FM_INTER,			//InterFM 76.5MHz(横浜中継局) 東京の親送信所は89.7MHｚ
		FM_JWAVE,			//J-WAVE 81.3MHz
		FM_NACK5,			//NACK5 79.5MHz
		FM_BUNKA,			//文化放送 91.6MHz
		FM_NIPPON,			//ニッポン放送 93.0MHz
};
const char* const RadioList[8] = {
		"FMyokohama\r\n",
		"TOKYO FM\r\n",
		"BAYFM\r\n",
		"interFM\r\n",
		"J-WAVE\r\n",
		"NACK5\r\n",
		"BUNKA HOUSOU\r\n",
		"NIPPON HOUSOU\r\n",
};
void SerialHome(UART_HandleTypeDef *huart,I2C_HandleTypeDef *hi2c,char *key)
{
	static char message[100];
	uint16_t GetFreq;
	uint8_t GetRssi;
	uint8_t key_int;

	sprintf(message,"YouPush...%c\r\n",*key);
	HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);

	switch(*key)
	{
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
		key_int = *key - '0';
		RadioTune(hi2c, RadioStationList[key_int]);
		strcpy(message,RadioList[key_int]);
		break;
	case 'S':
		Seek(hi2c, FLG_SEEKUP);
		strcpy(message,"SeekUp!\r\n");
		break;
	case 's':
		Seek(hi2c, FLG_SEEKDOWN);
		strcpy(message,"SeekDown!\r\n");
		break;
	case 'f':
		GetFreq = GetChan(hi2c);
		message[0] = GetFreq / 100 + '0';
		message[1] = (GetFreq / 10) % 10 + '0';
		message[2] = '.';
		message[3] = GetFreq % 10 + '0';

		memmove(message+4,"MHz\r\n",5);			//xx.xMHz
		break;
	case 'r':
		GetRssi = GetRSSI(hi2c);
		sprintf(message,"RSSI:%d\r\n",GetRssi);
		break;
	case 'V':
		if(SetVolume(hi2c, VolUp) == Failed)
		{
			strcpy(message,"MaxVolume!\r\n");
		}
		else
		{
			strcpy(message,"VolumeUp!\r\n");
		}
		break;
	case 'v':
		if(SetVolume(hi2c, VolDown) == Failed)
		{
			strcpy(message,"MinVolume!\r\n");
		}
		else
		{
			strcpy(message,"VolumeDown!\r\n");
		}
		break;
	case 'm':
		SetMute(hi2c);
		strcpy(message,"Mute!\r\n");
		break;
	case 'M':
		ResetMute(hi2c);
		strcpy(message,"unMute!\r\n");
		break;
	case '?':
		CommandManual(huart);
		break;
	}
	HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

void CommandManual(UART_HandleTypeDef *huart)
{
	char Help[100];

	strcpy(Help," 0~7 RadioStation\r\n S/s:SeekUp/Down\r\n f:GetFreq\r\n r:GetRSSI\r\n V/v:VolumeUp/Down\r\n M/m:MuteOn/OFF\r\n");

	HAL_UART_Transmit(huart, (uint8_t*)Help, strlen(Help), HAL_MAX_DELAY);
}
