#include "fan.h"
#include "tim.h"
#include "gpio.h"

void Fan_Init(void)
{
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);

    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
}

void Fan_SetSpeed(uint16_t speed)
{
    if(speed > 1000) speed = 1000;
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,speed);
}

void Fan_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,0);
}
