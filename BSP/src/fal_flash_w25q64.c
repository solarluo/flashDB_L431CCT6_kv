#include "fal.h"
#include "w25q64.h"

#define W25Q64_FLASH_SIZE   (8 * 1024 * 1024)
#define W25Q64_SECTOR_SIZE  4096

static int init(void)
{
    W25Q64_Init();
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    W25Q64_Read(buf, offset, size);
    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    W25Q64_Write((uint8_t *)buf, offset, size);
    return size;
}

static int erase(long offset, size_t size)
{
    while (size > 0)
    {
        W25Q64_EraseSector(offset);
        offset += W25Q64_SECTOR_SIZE;

        if (size > W25Q64_SECTOR_SIZE)
            size -= W25Q64_SECTOR_SIZE;
        else
            size = 0;
    }
    return 0;
}

const struct fal_flash_dev nor_flash0 =
{
    .name       = "w25q64",
    .addr       = 0,
    .len        = W25Q64_FLASH_SIZE,
    .blk_size   = W25Q64_SECTOR_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};