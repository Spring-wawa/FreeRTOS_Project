#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f1xx_hal.h"   // 根据你的芯片型号调整
#include <stdint.h>

/* 引脚定义（你的硬件接的是 PA7） */
#define DHT11_PORT   GPIOA
#define DHT11_PIN    GPIO_PIN_7

/* 温湿度数据结构体 */
typedef struct {
    uint8_t humidity;     // 湿度整数部分
    int8_t  temperature;  // 温度整数部分（DHT11 范围 0~50）
} DHT11_Data_t;

/**
  * @brief  上电初始化 DHT11 引脚（开漏输出，空闲高电平）
  */
void DHT11_Init(void);

/**
  * @brief  读取 DHT11 温湿度
  * @param  data : 存放结果的结构体指针
  * @retval 1: 读取成功, 0: 读取失败（无应答或校验错）
  */
uint8_t DHT11_Read(DHT11_Data_t *data);

#endif
