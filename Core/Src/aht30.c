#include "aht30.h"

static uint8_t AHT30_CalcCRC8(uint8_t *message, uint8_t num)
{
    uint8_t i;
    uint8_t byte;
    uint8_t crc = 0xFF;

    for (byte = 0; byte < num; byte++)
    {
        crc ^= message[byte];
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}
HAL_StatusTypeDef AHT30_IsReady(void)
{
    if (HAL_I2C_IsDeviceReady(&hi2c2, AHT30_I2C_ADDR, 3, 100) == HAL_OK)
        return HAL_OK;
    else
        return HAL_ERROR;
}
HAL_StatusTypeDef AHT30_Init(void)
{
    HAL_Delay(40);
    return HAL_OK;
}

HAL_StatusTypeDef AHT30_Read(float *temperature, float *humidity)
{
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    uint8_t data[7];
    uint32_t raw_humi, raw_temp;

    if (temperature == NULL || humidity == NULL)
        return HAL_ERROR;

    if (HAL_I2C_Master_Transmit(&hi2c2, AHT30_I2C_ADDR, cmd, 3, 100) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(80);

    if (HAL_I2C_Master_Receive(&hi2c2, AHT30_I2C_ADDR, data, 7, 100) != HAL_OK)
        return HAL_ERROR;

    if (data[0] & 0x80)
        return HAL_BUSY;

    if (AHT30_CalcCRC8(data, 6) != data[6])
        return HAL_ERROR;

    raw_humi = ((uint32_t)data[1] << 12) |
               ((uint32_t)data[2] << 4)  |
               ((uint32_t)(data[3] >> 4));

    raw_temp = (((uint32_t)data[3] & 0x0F) << 16) |
               ((uint32_t)data[4] << 8) |
               ((uint32_t)data[5]);

    *humidity    = ((float)raw_humi * 100.0f) / 1048576.0f;
    *temperature = ((float)raw_temp * 200.0f) / 1048576.0f - 50.0f;

    return HAL_OK;
}