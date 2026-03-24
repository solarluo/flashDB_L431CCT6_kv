#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include "fal_def.h"

extern const struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE             { &nor_flash0 }

/* 告诉 FAL：我使用静态分区表配置 */
#define FAL_PART_HAS_TABLE_CFG

/* 这两个宏，这个版本 fal_partition.c 还会检查 */
#define FAL_PART_TABLE_FLASH_DEV_NAME   "w25q64"
#define FAL_PART_TABLE_END_OFFSET       0

/* static partition table */
#ifdef FAL_PART_HAS_TABLE_CFG
#define FAL_PART_TABLE                                           \
{                                                                \
    {FAL_PART_MAGIC_WORD, "fdb_kv", "w25q64", 0, 256 * 1024, 0}, \
}
#endif

#endif /* _FAL_CFG_H_ */