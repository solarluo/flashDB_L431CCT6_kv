#ifndef __AHT30_H
#define __AHT30_H

#include "main.h"
#include <stdint.h>

extern I2C_HandleTypeDef hi2c2;

#define AHT30_I2C_ADDR   (0x38 << 1)

HAL_StatusTypeDef AHT30_Init(void);
HAL_StatusTypeDef AHT30_Read(float *temperature, float *humidity);
static uint8_t AHT30_CalcCRC8(uint8_t *message, uint8_t num);
HAL_StatusTypeDef AHT30_IsReady(void);
#endif