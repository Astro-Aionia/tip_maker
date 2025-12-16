#ifndef __EEPROM_EMULATION_H__
#define __EEPROM_EMULATION_H__

#include "stdint.h"

// 函数声明
void EE_Init(void);
uint16_t EE_ReadVariable(uint16_t virt_address, uint32_t* data);
uint16_t EE_WriteVariable(uint16_t virt_address, uint32_t data);

#endif /* __EEPROM_EMULATION_H__ */