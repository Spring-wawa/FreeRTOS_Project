#include "dht11.h"

/* 微秒级延时（72MHz，9次空循环 ≈ 1μs） */
static void Delay_us(uint32_t us) {
    uint32_t count = us * 9;
    while (count--) {
        __NOP();
    }
}

/* 配置引脚为推挽输出 */
static void DHT11_Mode_Out(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/* 配置引脚为上拉输入 */
static void DHT11_Mode_In(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
 * @brief  上电初始化（在 main 中调用一次）
 */
void DHT11_Init(void) {
    DHT11_Mode_Out();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);  // 空闲高电平
}

/**
 * @brief  读取 DHT11 温湿度
 * @param  data : 存放结果的结构体
 * @retval 1: 成功, 0: 失败
 */
uint8_t DHT11_Read(DHT11_Data_t *data) {
    int8_t i, j;              // *** 关键修改：j 改为 int8_t，消除警告 ***
    uint32_t cnt;
    uint8_t buff[5] = {0};

    /* 1. 主机起始信号 */
    DHT11_Mode_Out();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);                          // > 18ms
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    Delay_us(30);                           // 释放总线 30μs

    /* 2. 等待 DHT11 应答 */
    DHT11_Mode_In();

    // 等待低电平
    cnt = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
        cnt++;
        Delay_us(1);
        if (cnt > 500) return 0;
    }
    // 等待高电平
    cnt = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
        cnt++;
        Delay_us(1);
        if (cnt > 500) return 0;
    }

    /* 3. 读取 40bit */
    for (i = 0; i < 5; i++) {
        for (j = 7; j >= 0; j--) {          // 高位优先
            // 等待低电平（数据位开始）
            cnt = 0;
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
                cnt++;
                Delay_us(1);
                if (cnt > 500) return 0;
            }
            // 等待高电平
            cnt = 0;
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {
                cnt++;
                Delay_us(1);
                if (cnt > 500) return 0;
            }
            // 高电平开始后延时 40μs 判断
            Delay_us(40);
            if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
                buff[i] |= (1 << j);        // 读 1
            } else {
                buff[i] &= ~(1 << j);       // 读 0
            }
        }
    }

    /* 4. 校验 */
    if (buff[0] + buff[1] + buff[2] + buff[3] == buff[4]) {
        data->humidity = buff[0];
        data->temperature = buff[2];
        return 1;
    }
    return 0;
}
