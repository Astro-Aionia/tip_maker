#include "ina236.h"
#include "hal_instances.h"
#include "system_state.h"
#include <string.h>

// INA236电流校准参数
#define CURRENT_LSB_NANO 250 //最大电流为8192uA
#define SHUNT_CAL 621 //测试电阻为33Ω 

bool INA236_Init(void) {
    g_system_state.ina236_init_stat = false;

    // 设置配置寄存器
    uint8_t config_data[3] = {0};
    
    // 配置寄存器：0x4025
    config_data[0] = INA236_REG_CONFIG;
    config_data[1] = 0b01000000;  // 高字节
    config_data[2] = 0b00100101;  // 低字节
    
    if (HAL_I2C_Master_Transmit(&hi2c1, INA236_ADDRESS, 
                               config_data, 3, 100) != HAL_OK) {
        return false;
    }
    
    // 设置校准寄存器
    uint8_t cal_data[3] = {0};
    
    // 配置寄存器：SHUNT_CAL
    cal_data[0] = INA236_REG_CALIBRATION;
    cal_data[1] = (uint8_t)(SHUNT_CAL >> 8);   // 高字节
    cal_data[2] = (uint8_t)(SHUNT_CAL & 0xFF); // 低字节
    
    if (HAL_I2C_Master_Transmit(&hi2c1, INA236_ADDRESS, 
                            cal_data, 3, 100) != HAL_OK) {
        return false;
    }
    
    g_system_state.ina236_init_stat = true;
    return true;
}

bool INA236_ReadCurrent(uint16_t *current) {
    g_system_state.ina236_read_stat = true;

    if (!g_system_state.ina236_init_stat) return false;
    
    uint8_t reg_addr[1] = {INA236_REG_SHUNT_VOLT};
    uint8_t read_data[2] = {0};
    
    // 写入要读取的寄存器地址
    if (HAL_I2C_Master_Transmit(&hi2c1, INA236_ADDRESS, 
                               &reg_addr, 1, 100) != HAL_OK) {
        g_system_state.ina236_read_stat = true;
        return false;
    }
    
    // 读取电流数据
    if (HAL_I2C_Master_Receive(&hi2c1, INA236_ADDRESS, 
                              read_data, 2, 10) != HAL_OK) {
        g_system_state.ina236_read_stat = false;
        return false;
    }
    
    // 转换为有符号16位整数
    int16_t raw_current = (read_data[0] << 8) | read_data[1];
    
    // 转换为实际电流值（安培）
    *current = raw_current * CURRENT_LSB_NANO / 1000; // 转换为微安培单位
    
    return true;
}