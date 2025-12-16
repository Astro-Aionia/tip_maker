#include "ina236.h"
#include "hal_instances.h"
#include "system_state.h"
#include <string.h>

#define CURRENT_LSB 0.00000025f
#define SHUNT_CAL_VALUE 5.0f

static bool ina236_initialized = false;

bool INA236_Init(void) {
    uint8_t config_data[3] = {0};
    
    // 配置寄存器：0x8127
    config_data[0] = INA236_REG_CONFIG;
    config_data[1] = 0x27;  // 低字节
    config_data[2] = 0x81;  // 高字节
    
    if (HAL_I2C_Master_Transmit(&hi2c1, INA236_ADDRESS << 1, 
                               config_data, 3, 100) != HAL_OK) {
        return false;
    }
    
    // 设置校准寄存器
    INA236_SetCalibration();
    
    ina236_initialized = true;
    return true;
}

void INA236_SetCalibration(void) {
    uint8_t cal_data[3] = {0};
    uint16_t cal_value;
    
    // 计算校准值
    cal_value = (uint16_t)(0.00512 / (CURRENT_LSB * SHUNT_CAL_VALUE));
    
    cal_data[0] = INA236_REG_CALIBRATION;
    cal_data[1] = cal_value & 0xFF;        // 低字节
    cal_data[2] = (cal_value >> 8) & 0xFF; // 高字节
    
    HAL_I2C_Master_Transmit(&hi2c1, INA236_ADDRESS << 1, cal_data, 3, 100);
}

bool INA236_ReadCurrent(float *current) {
    if (!ina236_initialized) return false;
    
    uint8_t reg_addr = INA236_REG_CURRENT;
    uint8_t read_data[2] = {0};
    
    // 写入要读取的寄存器地址
    if (HAL_I2C_Master_Transmit(&hi2c1, INA236_ADDRESS << 1, 
                               &reg_addr, 1, 10) != HAL_OK) {
        return false;
    }
    
    // 读取电流数据
    if (HAL_I2C_Master_Receive(&hi2c1, INA236_ADDRESS << 1, 
                              read_data, 2, 10) != HAL_OK) {
        return false;
    }
    
    // 转换为有符号16位整数
    int16_t raw_current = (read_data[0] << 8) | read_data[1];
    
    // 转换为实际电流值（安培）
    *current = raw_current * CURRENT_LSB;
    
    return true;
}