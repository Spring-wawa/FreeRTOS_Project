# STM32 温控风扇控制系统（FreeRTOS）

## 1. 项目简介
本项目基于 STM32F103 + FreeRTOS 实现温度采集与风扇自动调速控制。
通过 ADC 采集温度传感器数据，根据温度分档控制 PWM 输出，实现风扇转速调节。

---

## 2. 功能说明
- 实时温度采集（ADC）
- PWM 控制风扇转速
- FreeRTOS 多任务调度
- 串口数据输出调试
- 温度分段控制（带滞回防抖）

---

## 3. 硬件平台
- MCU：STM32F103C8T6
- 温度传感器：NTC / LM35（根据实际）
- 风扇：PWM 控制直流风扇
- 调试：USART1

---

## 4. 软件架构
- FreeRTOS
  - Temperature Task（温度采集）
  - Control Task（风扇控制）
  - UART Task（数据输出）

---

## 5. 工程结构
Core/
├── Src/
├── Inc/
Drivers/
Middlewares/
FreeRTOS/

---

## 6. 控制逻辑
- ≥35℃：100%转速
- 34℃：70%转速
- 33℃：40%转速
- <32℃：停止或低速

（带滞回机制，防止频繁跳变）

---

## 7. 使用方法
1. 使用 Keil 打开工程
2. 编译下载到 STM32
3. 串口查看温度数据
4. 调节温度源测试风扇变化

---

## 8. 关键实现
- PWM 使用 TIM3
- ADC DMA 采样
- FreeRTOS 任务调度
- 滞回控制算法

---

## 9. 作者
chen spring  
Version: v1.0
