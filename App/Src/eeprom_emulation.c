#include "eeprom_emulation.h"
#include "stm32f1xx_hal.h"

// Flash配置
#define FLASH_PAGE_SIZE    1024
#define FLASH_END_ADDRESS  0x0801FC00  // 最后一页的起始地址（64KB Flash的最后一页）
#define EEPROM_START_ADDRESS FLASH_END_ADDRESS
#define EEPROM_SIZE         FLASH_PAGE_SIZE

// 虚拟地址到物理地址的转换
#define VIRTUAL_ADDR_TO_PHYSICAL(addr) (EEPROM_START_ADDRESS + (addr) * 4)

void EE_Init(void) {
    // 初始化Flash接口
    HAL_FLASH_Unlock();
}

uint16_t EE_ReadVariable(uint16_t virt_address, uint32_t* data) {
    uint32_t address = VIRTUAL_ADDR_TO_PHYSICAL(virt_address);
    
    // 检查地址是否有效
    if (address >= EEPROM_START_ADDRESS + EEPROM_SIZE) {
        return 1; // 错误：地址超出范围
    }
    
    // 读取数据
    *data = *(__IO uint32_t*)address;
    
    // 检查是否已写入（不是0xFFFFFFFF）
    if (*data == 0xFFFFFFFF) {
        return 2; // 错误：变量未初始化
    }
    
    return 0; // 成功
}

uint16_t EE_WriteVariable(uint16_t virt_address, uint32_t data) {
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error;
    uint32_t address = VIRTUAL_ADDR_TO_PHYSICAL(virt_address);
    
    // 检查地址是否有效
    if (address >= EEPROM_START_ADDRESS + EEPROM_SIZE) {
        return 1; // 错误：地址超出范围
    }
    
    // 检查是否与当前值相同
    if (*(__IO uint32_t*)address == data) {
        return 0; // 已是最新值
    }
    
    // 如果当前页需要擦除
    if (*(__IO uint32_t*)address != 0xFFFFFFFF) {
        // 擦除整个页面
        erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
        erase_init.PageAddress = EEPROM_START_ADDRESS;
        erase_init.NbPages = 1;
        
        if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK) {
            return 3; // 错误：擦除失败
        }
    }
    
    // 写入数据
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data) != HAL_OK) {
        return 4; // 错误：写入失败
    }
    
    return 0; // 成功
}