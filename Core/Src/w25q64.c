#include "w25q64.h"
#include "spi.h"

#define W25_CS_PORT   GPIOA
#define W25_CS_PIN    GPIO_PIN_4

#define W25_CS_LOW()   HAL_GPIO_WritePin(W25_CS_PORT, W25_CS_PIN, GPIO_PIN_RESET)
#define W25_CS_HIGH()  HAL_GPIO_WritePin(W25_CS_PORT, W25_CS_PIN, GPIO_PIN_SET)

void W25Q64_Init(void)
{
    W25_CS_HIGH();
}

uint8_t W25Q64_ReadWriteByte(uint8_t txData)
{
    uint8_t rxData = 0;
    HAL_SPI_TransmitReceive(&hspi1, &txData, &rxData, 1, HAL_MAX_DELAY);
    return rxData;
}

void W25Q64_ReadJEDECID(uint8_t *mid, uint8_t *type, uint8_t *capacity)
{
    W25_CS_LOW();

    W25Q64_ReadWriteByte(0x9F);
    *mid      = W25Q64_ReadWriteByte(0xFF);
    *type     = W25Q64_ReadWriteByte(0xFF);
    *capacity = W25Q64_ReadWriteByte(0xFF);

    W25_CS_HIGH();
}

/* 写使能 */
void W25Q64_WriteEnable(void)
{
    W25_CS_LOW();
    W25Q64_ReadWriteByte(0x06);
    W25_CS_HIGH();
}

/* 等待忙结束 */
void W25Q64_WaitBusy(void)
{
    uint8_t status;

    W25_CS_LOW();
    W25Q64_ReadWriteByte(0x05);   // 读状态寄存器1
    do
    {
        status = W25Q64_ReadWriteByte(0xFF);
    } while (status & 0x01);
    W25_CS_HIGH();
}

/* 读取任意长度数据 */
void W25Q64_Read(uint8_t *buf, uint32_t addr, uint32_t size)
{
    uint32_t i;

    W25_CS_LOW();

    W25Q64_ReadWriteByte(0x03);   // Read Data
    W25Q64_ReadWriteByte((addr >> 16) & 0xFF);
    W25Q64_ReadWriteByte((addr >> 8) & 0xFF);
    W25Q64_ReadWriteByte(addr & 0xFF);

    for (i = 0; i < size; i++)
    {
        buf[i] = W25Q64_ReadWriteByte(0xFF);
    }

    W25_CS_HIGH();
}

/* 4KB 扇区擦除 */
void W25Q64_EraseSector(uint32_t addr)
{
    W25Q64_WriteEnable();

    W25_CS_LOW();

    W25Q64_ReadWriteByte(0x20);   // Sector Erase 4KB
    W25Q64_ReadWriteByte((addr >> 16) & 0xFF);
    W25Q64_ReadWriteByte((addr >> 8) & 0xFF);
    W25Q64_ReadWriteByte(addr & 0xFF);

    W25_CS_HIGH();

    W25Q64_WaitBusy();
}

/* 页写，自动处理跨页 */
void W25Q64_Write(uint8_t *buf, uint32_t addr, uint32_t size)
{
    uint32_t page_remain;
    uint32_t write_size;
    uint32_t i;

    while (size > 0)
    {
        page_remain = 256 - (addr % 256);
        write_size = (size < page_remain) ? size : page_remain;

        W25Q64_WriteEnable();

        W25_CS_LOW();

        W25Q64_ReadWriteByte(0x02);   // Page Program
        W25Q64_ReadWriteByte((addr >> 16) & 0xFF);
        W25Q64_ReadWriteByte((addr >> 8) & 0xFF);
        W25Q64_ReadWriteByte(addr & 0xFF);

        for (i = 0; i < write_size; i++)
        {
            W25Q64_ReadWriteByte(buf[i]);
        }

        W25_CS_HIGH();

        W25Q64_WaitBusy();

        addr += write_size;
        buf  += write_size;
        size -= write_size;
    }
}