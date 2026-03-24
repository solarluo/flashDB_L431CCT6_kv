#ifndef __W25Q64_H
#define __W25Q64_H

#include "main.h"

void W25Q64_Init(void);
uint8_t W25Q64_ReadWriteByte(uint8_t txData);
void W25Q64_ReadJEDECID(uint8_t *mid, uint8_t *type, uint8_t *capacity);

void W25Q64_WriteEnable(void);
void W25Q64_WaitBusy(void);
void W25Q64_Read(uint8_t *buf, uint32_t addr, uint32_t size);
void W25Q64_Write(uint8_t *buf, uint32_t addr, uint32_t size);
void W25Q64_EraseSector(uint32_t addr);

#endif