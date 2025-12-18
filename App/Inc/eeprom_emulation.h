#ifndef __EEPROM_EMULATION_H__
#define __EEPROM_EMULATION_H__

#include "stdint.h"

// 虚拟地址定义
#define EE_ADDR_FREQ        0x0001
#define EE_ADDR_THRES       0x0002
#define EE_ADDR_DEBUG_LEVEL 0x0003

// 函数声明
void EE_Init(void);
uint16_t EE_ReadVariable(uint16_t virt_address, uint32_t* data);
uint16_t EE_WriteVariable(uint16_t virt_address, uint32_t data);

#endif /* __EEPROM_EMULATION_H__ */