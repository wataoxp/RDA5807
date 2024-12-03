/*
 * Radio.c
 *
 *  Created on: Noc 30, 2024
 *      Author: wataoxp
 */
#include "Radio.h"
#include "main.h"

#define RDA5807M

typedef struct{
	uint16_t Config;
	uint16_t Tuning;
	uint16_t GpioSet;
	uint16_t VolumeSet;
}RDA_Typedef;

static RDA_Typedef RadioReg = {0};

static inline void SetRegister(I2C_HandleTypeDef *hi2c,uint8_t reg,uint16_t value)
{
	uint8_t WriteBuffer[2] ={
			(value >> 8),(value & 0xFF),
	};
	HAL_I2C_Mem_Write(hi2c, RADIO_ADDR_PUSH, reg, I2C_MEMADD_SIZE_8BIT, WriteBuffer, sizeof(WriteBuffer), HAL_MAX_DELAY);
}
#ifdef RDA5807M
void RadioInit(I2C_HandleTypeDef *hi2c)
{
	RadioReg.Config = FLG_DHIZ | FLG_DMUTE | FLG_BASS | FLG_NEW | FLG_ENABLE;
	RadioReg.Tuning = BAND_WORLD;
	RadioReg.GpioSet = FLG_DE | FLG_SOFT_MUTE;
	RadioReg.VolumeSet = FLG_LNA | FLG_INT_ENABLE | (0x4 << THRESHOLD_SHIFT) | VOLUME_MASK;

	uint8_t init[] = {
			(RadioReg.Config >> 8),(RadioReg.Config & 0xFF),
			(RadioReg.Tuning >> 8),(RadioReg.Tuning & 0xFF),
			(RadioReg.GpioSet >> 8),(RadioReg.GpioSet & 0xFF),
			(RadioReg.VolumeSet >> 8),(RadioReg.VolumeSet & 0xFF),
	};
	HAL_I2C_Master_Transmit(hi2c, RADIO_ADDR_SEQ, init, sizeof(init), HAL_MAX_DELAY);
}
void Seek(I2C_HandleTypeDef *hi2c ,uint16_t seekmode)
{
	uint8_t stc = 0;

	RadioReg.Config = (RadioReg.Config & ~(FLG_SEEKUP | FLG_SEEK)) | FLG_SEEK | seekmode;

	SetRegister(hi2c, REG_CONFIG,RadioReg.Config);

	while(!stc)										//Wait for 0AH 14bit Set
	{
		stc = GetRegister(hi2c, REG_STATUS) >> STC_SHIFT;
		HAL_Delay(5);
	}
}

#else	/* Use RDA5807FP GPIOinterrupt */
void RadioInit(I2C_HandleTypeDef *hi2c)
{
	RadioReg.Config = FLG_DHIZ | FLG_DMUTE | FLG_BASS | FLG_NEW | FLG_ENABLE;
	RadioReg.Tuning = BAND_WORLD;
	RadioReg.GpioSet = FLG_DE | FLG_STC | FLG_SOFT_MUTE | GPIO2_INT_OFF;
	RadioReg.VolumeSet = FLG_LNA | FLG_INT_ENABLE | (0x4 << THRESHOLD_SHIFT) | VOLUME_MASK;

	uint8_t init[] = {
			(RadioReg.Config >> 8),(RadioReg.Config & 0xFF),
			(RadioReg.Tuning >> 8),(RadioReg.Tuning & 0xFF),
			(RadioReg.GpioSet >> 8),(RadioReg.GpioSet & 0xFF),
			(RadioReg.VolumeSet >> 8),(RadioReg.VolumeSet & 0xFF),
	};
	HAL_I2C_Master_Transmit(hi2c, RADIO_ADDR_SEQ, init, sizeof(init), HAL_MAX_DELAY);
}
void Seek(I2C_HandleTypeDef *hi2c ,uint16_t seekmode)
{
	RadioReg.GpioSet = (RadioReg.GpioSet & ~GPIO2_INT_MASK) | GPIO2_INT_ON;
	RadioReg.Config = (RadioReg.Config & ~(FLG_SEEKUP | FLG_SEEK)) | FLG_SEEK | seekmode;

	SetRegister(hi2c, REG_GPIO,RadioReg.GpioSet);
	SetRegister(hi2c, REG_CONFIG,RadioReg.Config);
	while(HAL_GPIO_ReadPin(STC_GPIO_Port, STC_Pin) != GPIO_PIN_RESET);		//Wait for GPIO2 Falling

	//After the seek is completed GPIO2 Reset
	RadioReg.GpioSet = (RadioReg.GpioSet & ~GPIO2_INT_MASK) | GPIO2_INT_OFF;
	SetRegister(hi2c, REG_GPIO, RadioReg.GpioSet);
}
#endif

void RadioTune(I2C_HandleTypeDef *hi2c,uint16_t chan)
{
	RadioReg.Tuning = (RadioReg.Tuning & ~(CHAN_WRITE_MASK | FLG_TUNE)) | (chan << CHAN_SHIFT) | FLG_TUNE;

	SetRegister(hi2c, REG_TUNING, RadioReg.Tuning);
}

const uint8_t VolumeParam[4] ={
		0x01,0x03,0x07,0x0F,
};
Radio_bool SetVolume(I2C_HandleTypeDef *hi2c,uint8_t dir)
{
	static int8_t count = (sizeof(VolumeParam) - 1);

	if(dir == VolDown)
	{
		count--;
		if(count < 0)
		{
			return Failed;
		}
	}
	else if(dir == VolUp)
	{
		count++;
		if(count > 3)
		{
			return Failed;
		}
	}
	RadioReg.VolumeSet = (RadioReg.VolumeSet & ~VOLUME_MASK) | (VolumeParam[count] & VOLUME_MASK);
	SetRegister(hi2c, REG_VOLUME, RadioReg.VolumeSet);

	return Success;

}
void SetMute(I2C_HandleTypeDef *hi2c)
{
	RadioReg.Config = (RadioReg.Config & ~FLG_DMUTE);
	SetRegister(hi2c, REG_CONFIG, RadioReg.Config);
}
void ResetMute(I2C_HandleTypeDef *hi2c)
{
	RadioReg.Config = (RadioReg.Config & ~FLG_DMUTE) | FLG_DMUTE;
	SetRegister(hi2c, REG_CONFIG, RadioReg.Config);
}
uint16_t GetRegister(I2C_HandleTypeDef *hi2c,uint8_t Reg)
{
	uint8_t Read[2];
	uint16_t ReadValue;
	HAL_I2C_Mem_Read(hi2c, RADIO_ADDR_PUSH, Reg, I2C_MEMADD_SIZE_8BIT, Read, sizeof(Read), HAL_MAX_DELAY);

	ReadValue = (Read[0] << 8) | (Read[1] & 0xFF);
	return ReadValue;
}
uint16_t GetChan(I2C_HandleTypeDef *hi2c)
{
	uint16_t ReadFreq;

	//実数ではなく整数として取得する
	//Get it as an integer instead of a real number
	ReadFreq = GetRegister(hi2c, REG_STATUS);
	ReadFreq &= CHAN_READ_MASK;
	ReadFreq += 760;

	return ReadFreq;
}
uint8_t GetRSSI(I2C_HandleTypeDef *hi2c)
{
	uint16_t ReadRssi;

	ReadRssi = GetRegister(hi2c, REG_RSSI);
	ReadRssi &= RSSI_MASK;
	ReadRssi >>= RSSI_SHIFT;

	return (uint8_t)ReadRssi;
}
