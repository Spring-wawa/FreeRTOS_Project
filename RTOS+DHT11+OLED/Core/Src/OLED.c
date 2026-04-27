#include "oled.h"
#include "i2c.h"                   /* 提供 hi2c1 的声明 */
#include "OLED_Font.h"             /* 字模库，保持不变 */

extern I2C_HandleTypeDef hi2c1;    /* CubeMX 生成的 I2C 句柄 */

/*================================ 底层硬件接口（HAL 硬件 I2C） ================================*/

/**
  * @brief  向 OLED 写命令
  * @param  Command 命令字节
  */
static void OLED_WriteCommand(uint8_t Command)
{
    /* 从机地址 0x78 (7位地址 0x3C << 1)，控制字节 0x00 表示命令 */
    HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT, &Command, 1, 100);
}

/**
  * @brief  向 OLED 写数据
  * @param  Data 数据字节
  */
static void OLED_WriteData(uint8_t Data)
{
    /* 控制字节 0x40 表示数据 */
    HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x40, I2C_MEMADD_SIZE_8BIT, &Data, 1, 100);
}

/**
  * @brief  设置 OLED 的光标位置（页寻址模式）
  * @param  Y 页地址，0~7
  * @param  X 列地址，0~127
  */
static void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                  // 设置页地址
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));  // 设置列地址高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));         // 设置列地址低4位
}

/*================================ 功能函数（完全沿用标准库逻辑） ================================*/

/**
  * @brief  OLED 清屏
  */
void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for(i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

/**
  * @brief  OLED 显示一个字符（8x16 点阵，支持 ASCII 可见字符）
  * @param  Line   行号，1 ~ 4
  * @param  Column 列号，1 ~ 16
  * @param  Char   显示的字符（' ' ~ '~'）
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    uint8_t y = (Line - 1) * 2;        // 每行占两页
    uint8_t x = (Column - 1) * 8;      // 每列8像素

    OLED_SetCursor(y, x);              // 上半页
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }

    OLED_SetCursor(y + 1, x);          // 下半页
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
    }
}

/**
  * @brief  OLED 显示字符串
  * @param  Line   起始行（1~4）
  * @param  Column 起始列（1~16）
  * @param  String 字符串指针
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/**
  * @brief  计算 X 的 Y 次方（内部使用）
  */
static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
  * @brief  OLED 显示十进制数字（正数）
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED 显示带符号十进制数字
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED 显示十六进制数字（正数）
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

/**
  * @brief  OLED 显示二进制数字（正数）
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/**
  * @brief  OLED 初始化（HAL 硬件 I2C 版本）
  */
void OLED_Init(void)
{
    /* 等待上电稳定（HAL 延时替代软件空循环） */
    HAL_Delay(100);
    
    /* I2C 初始化由 CubeMX 生成的 MX_I2C1_Init() 完成，这里不再重复 */

    OLED_WriteCommand(0xAE);   // 关闭显示

    OLED_WriteCommand(0x20);   // 设置内存寻址模式
    OLED_WriteCommand(0x02);   // 页寻址模式 （标准库原版无此配置，但建议保留）

    OLED_WriteCommand(0xD5);   // 设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);

    OLED_WriteCommand(0xA8);   // 多路复用率
    OLED_WriteCommand(0x3F);

    OLED_WriteCommand(0xD3);   // 显示偏移
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x40);   // 显示起始行

    OLED_WriteCommand(0xA1);   // 左右方向（正常）

    OLED_WriteCommand(0xC8);   // 上下方向（正常）

    OLED_WriteCommand(0xDA);   // COM 引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81);   // 对比度
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9);   // 预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);   // VCOMH 电平
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);   // 全局显示开启

    OLED_WriteCommand(0xA6);   // 正常显示（非反白）

    OLED_WriteCommand(0x8D);   // 启用电荷泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);   // 开启显示

    OLED_Clear();              // 清屏
}
