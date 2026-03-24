#include "st7735.h"

/*-------------------- 引脚定义 --------------------*/
/* 按你当前工程的引脚来 */
#define LCD_CS_LOW()      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define LCD_CS_HIGH()     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)

#define LCD_DC_LOW()      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define LCD_DC_HIGH()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)

#define LCD_RST_LOW()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)
#define LCD_RST_HIGH()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)

#define LCD_BL_LOW()      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET)
#define LCD_BL_HIGH()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET)

/*-------------------- 这块 0.96 寸屏的重要参数 --------------------*/
/* 厂家例程里已经体现：80x160，X 偏移 26，Y 偏移 1 */
static const uint16_t st7735_x_offset = 26;
static const uint16_t st7735_y_offset = 1;

/*-------------------- 底层函数 --------------------*/
static void ST7735_Reset(void)
{
    LCD_RST_HIGH();
    HAL_Delay(5);

    LCD_RST_LOW();
    HAL_Delay(20);

    LCD_RST_HIGH();
    HAL_Delay(120);
}

static void ST7735_WriteCommand(uint8_t cmd)
{
    LCD_CS_LOW();
    LCD_DC_LOW();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void ST7735_WriteData(const uint8_t *data, uint16_t size)
{
    LCD_CS_LOW();
    LCD_DC_HIGH();
    HAL_SPI_Transmit(&hspi2, (uint8_t *)data, size, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void ST7735_WriteDataByte(uint8_t data)
{
    LCD_CS_LOW();
    LCD_DC_HIGH();
    HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

/*-------------------- 设置显示窗口 --------------------*/
void ST7735_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t xs = x0 + st7735_x_offset;
    uint16_t xe = x1 + st7735_x_offset;
    uint16_t ys = y0 + st7735_y_offset;
    uint16_t ye = y1 + st7735_y_offset;

    uint8_t data[4];

    /* Column address set */
    ST7735_WriteCommand(0x2A);
    data[0] = (xs >> 8) & 0xFF;
    data[1] = xs & 0xFF;
    data[2] = (xe >> 8) & 0xFF;
    data[3] = xe & 0xFF;
    ST7735_WriteData(data, 4);

    /* Row address set */
    ST7735_WriteCommand(0x2B);
    data[0] = (ys >> 8) & 0xFF;
    data[1] = ys & 0xFF;
    data[2] = (ye >> 8) & 0xFF;
    data[3] = ye & 0xFF;
    ST7735_WriteData(data, 4);

    /* RAM write */
    ST7735_WriteCommand(0x2C);
}

/*-------------------- 画点 --------------------*/
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t data[2];

    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT)
        return;

    ST7735_SetAddressWindow(x, y, x, y);

    data[0] = (color >> 8) & 0xFF;
    data[1] = color & 0xFF;
    ST7735_WriteData(data, 2);
}

/*-------------------- 填充矩形 --------------------*/
void ST7735_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint8_t data[2];

    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT)
        return;

    if ((x + w) > ST7735_WIDTH)
        w = ST7735_WIDTH - x;

    if ((y + h) > ST7735_HEIGHT)
        h = ST7735_HEIGHT - y;

    ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    data[0] = (color >> 8) & 0xFF;
    data[1] = color & 0xFF;

    LCD_CS_LOW();
    LCD_DC_HIGH();

    for (uint32_t i = 0; i < (uint32_t)w * h; i++)
    {
        HAL_SPI_Transmit(&hspi2, data, 2, HAL_MAX_DELAY);
    }

    LCD_CS_HIGH();
}

/*-------------------- 整屏填充 --------------------*/
void ST7735_FillScreen(uint16_t color)
{
    ST7735_FillRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

/*-------------------- 初始化 --------------------*/
void ST7735_Init(void)
{
    /* 空闲状态 */
    LCD_CS_HIGH();
    LCD_DC_HIGH();
    LCD_RST_HIGH();

    /* 开背光 */
    LCD_BL_HIGH();

    /* 硬复位 */
    ST7735_Reset();

    /* Sleep Out */
    ST7735_WriteCommand(0x11);
    HAL_Delay(120);

    /* Memory Data Access Control
       厂家例程 USE_HORIZONTAL == 1 时用的是 0x08
       这一版先按厂家给的方向来
    */
    ST7735_WriteCommand(0x36);
    ST7735_WriteDataByte(0x08);

    /* Frame Rate Control */
    ST7735_WriteCommand(0xB1);
    {
        uint8_t data[] = {0x05, 0x3C, 0x3C};
        ST7735_WriteData(data, sizeof(data));
    }

    ST7735_WriteCommand(0xB2);
    {
        uint8_t data[] = {0x05, 0x3C, 0x3C};
        ST7735_WriteData(data, sizeof(data));
    }

    ST7735_WriteCommand(0xB3);
    {
        uint8_t data[] = {0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C};
        ST7735_WriteData(data, sizeof(data));
    }

    /* Dot inversion */
    ST7735_WriteCommand(0xB4);
    ST7735_WriteDataByte(0x03);

    /* Power Sequence - 按厂家例程 */
    ST7735_WriteCommand(0xC0);
    {
        uint8_t data[] = {0x0E, 0x0E, 0x04};
        ST7735_WriteData(data, sizeof(data));
    }

    ST7735_WriteCommand(0xC1);
    ST7735_WriteDataByte(0xC0);

    ST7735_WriteCommand(0xC2);
    {
        uint8_t data[] = {0x0D, 0x00};
        ST7735_WriteData(data, sizeof(data));
    }

    ST7735_WriteCommand(0xC3);
    {
        uint8_t data[] = {0x8D, 0x2A};
        ST7735_WriteData(data, sizeof(data));
    }

    ST7735_WriteCommand(0xC4);
    {
        uint8_t data[] = {0x8D, 0xEE};
        ST7735_WriteData(data, sizeof(data));
    }

    /* VCOM */
    ST7735_WriteCommand(0xC5);
    ST7735_WriteDataByte(0x04);

    /* 16-bit color */
    ST7735_WriteCommand(0x3A);
    ST7735_WriteDataByte(0x05);

    /* Gamma - 按厂家例程 */
    ST7735_WriteCommand(0xE0);
    {
        uint8_t data[] = {
            0x05, 0x1A, 0x0B, 0x15,
            0x3D, 0x38, 0x2E, 0x30,
            0x2D, 0x28, 0x30, 0x3B,
            0x00, 0x01, 0x02, 0x10
        };
        ST7735_WriteData(data, sizeof(data));
    }

    ST7735_WriteCommand(0xE1);
    {
        uint8_t data[] = {
            0x05, 0x1A, 0x0B, 0x15,
            0x36, 0x2E, 0x28, 0x2B,
            0x2B, 0x28, 0x30, 0x3B,
            0x00, 0x01, 0x02, 0x10
        };
        ST7735_WriteData(data, sizeof(data));
    }

/* Display inversion ON */
ST7735_WriteCommand(0x21);
HAL_Delay(10);

/* Normal display on */
ST7735_WriteCommand(0x13);
HAL_Delay(10);

/* Display on */
ST7735_WriteCommand(0x29);
HAL_Delay(20);


    /* 上电先清黑屏 */
    ST7735_FillScreen(ST7735_BLACK);
}
void ST7735_PushColors(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *colors)
{
    uint32_t pixel_count;
    uint32_t byte_count;

    if (colors == NULL) return;
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT) return;
    if ((x + w) > ST7735_WIDTH)  w = ST7735_WIDTH - x;
    if ((y + h) > ST7735_HEIGHT) h = ST7735_HEIGHT - y;
    if (w == 0 || h == 0) return;

    ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    pixel_count = (uint32_t)w * h;
    byte_count  = pixel_count * 2U;

    LCD_CS_LOW();
    LCD_DC_HIGH();

    /* 你的屏现在就是 RGB565 16-bit 模式 */
    HAL_SPI_Transmit(&hspi2, (uint8_t *)colors, byte_count, HAL_MAX_DELAY);

    LCD_CS_HIGH();
}
