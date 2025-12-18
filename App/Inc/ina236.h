#ifndef __INA236_H__
#define __INA236_H__

#include "stdint.h"
#include "stdbool.h"

// INA236地址
#define INA236_ADDRESS (uint16_t)0b10000000

// 寄存器地址
#define INA236_REG_CONFIG           (uint8_t)0x00
#define INA236_REG_SHUNT_VOLT       (uint8_t)0x01
#define INA236_REG_BUS_VOLT         (uint8_t)0x02
#define INA236_REG_POWER            (uint8_t)0x03
#define INA236_REG_CURRENT          (uint8_t)0x04
#define INA236_REG_CALIBRATION      (uint8_t)0x05
#define INA236_REG_MASK_ENABLE      (uint8_t)0x06
#define INA236_REG_ALERT_LIMIT      (uint8_t)0x07
#define INA236_REG_MANUFACTURER_ID  (uint8_t)0x3E
#define INA236_REG_DEVICE_ID        (uint8_t)0x3F

// 函数声明
bool INA236_Init(void);
bool INA236_ReadCurrent(uint16_t *current);

#endif /* __INA236_H__ */