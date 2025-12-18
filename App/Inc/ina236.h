#ifndef __INA236_H__
#define __INA236_H__

#include "stdint.h"
#include "stdbool.h"

// INA236地址
#define INA236_ADDRESS 0x40

// 寄存器地址
#define INA236_REG_CONFIG           0x00
#define INA236_REG_SHUNT_VOLT       0x01
#define INA236_REG_BUS_VOLT         0x02
#define INA236_REG_POWER            0x03
#define INA236_REG_CURRENT          0x04
#define INA236_REG_CALIBRATION      0x05
#define INA236_REG_MASK_ENABLE      0x06
#define INA236_REG_ALERT_LIMIT      0x07
#define INA236_REG_MANUFACTURER_ID  0x3E
#define INA236_REG_DEVICE_ID        0x3F

// 函数声明
bool INA236_Init(void);
bool INA236_ReadCurrent(uint16_t *current);
void INA236_SetCalibration(void);

#endif /* __INA236_H__ */