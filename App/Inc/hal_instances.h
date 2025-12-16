#ifndef __HAL_INSTANCES_H
#define __HAL_INSTANCES_H

#include "main.h" // 这个文件包含了所有HAL头文件和外设句柄的声明

/* 
 * 声明所有在 main.c 中定义的外设句柄为 extern。
 * 这样任何包含此头文件的 .c 文件都能使用它们。
 */
extern TIM_HandleTypeDef htim1;
extern I2C_HandleTypeDef hi2c1;
// 可以根据需要添加其他外设，如 SPI, ADC 等

#endif /* __HAL_INSTANCES_H */