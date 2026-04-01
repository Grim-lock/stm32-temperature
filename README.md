# stm32-temperature
温湿度计

## 1. 介绍
这是一个基于 STM32 微控制器的温湿度计项目，使用SHT20传感器进行温湿度测量，并通过OLED显示屏显示结果。

## 2. 硬件需求
- STM32 微控制器（如 STM32F103C8T6）
- i2c expanusion board
- SHT20 温湿度传感器 switch channal 2
- OLED 显示屏 ssd1306驱动 switch channal 1
- 连接线和电源

[ STM32F103C6/8T6 ]
      (Default Master / 主机)
               ||
               || (I2C1 总线)
      [PB6]---->>----[SCL]
      [PB7]---->>----[SDA]
               ||
               ||
      +--------------------------+
      |   I2C Expansion Board    |
      |      (TCA9548A)          |
      |    Slave Addr: 0x70      |
      +------------+-------------+
                   |
         /---------+---------\
         |                   |
   [ Channel 1 ]       [ Channel 2 ]
   (Control: 0x02)     (Control: 0x04)
         |                   |
   <<--[SC1/SD1]-->>   <<--[SC2/SD2]-->>
         |                   |
         |                   |
  [ OLED Display ]     [ SHT20 Sensor ]
  (SSD1306, 64x48)     (Temp/Humidity)
  Addr: 0x3C           Addr: 0x40