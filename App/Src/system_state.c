#include "system_state.h"
#include "eeprom_emulation.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include <stdio.h>

SystemState_t g_system_state;

// 脉冲计数完成回调函数
static void MotorPulseCompleteCallback(void) {
    // 当脉冲计数完成时更新系统状态
    g_system_state.current_steps = g_system_state.target_steps;
    g_system_state.motor_moving = false;
    
    // 如果启用了调试输出，可以在这里添加
    if (g_system_state.debug_enabled) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Motor move completed: %d steps\r\n", 
                g_system_state.current_steps);
        CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
    }
}

void SystemState_Init(void) {
    // 初始化固有参数
    //g_system_state.origin_freq = 1000;
    // g_system_state.freq = g_system_state.origin_freq;
    // g_system_state.threshold = 50;
    // 从EEPROM加载用户设置
    SystemState_LoadFromEEPROM();
    
    // 如果EEPROM中没有值，使用默认值
    /*if (g_system_state.freq > 1000) {
        g_system_state.freq = 500; // 默认500Hz
    }*/
    
    // 初始化开关状态
    g_system_state.switch_current = false;
    g_system_state.switch_holdoff = false;
    g_system_state.switch_division = false;
    
    // 初始化输入状态
    g_system_state.zero_point = false;
    g_system_state.round_count = 0;
    
    // 初始化电流缓冲区
    memset(g_system_state.current_buffer, 0, sizeof(g_system_state.current_buffer));
    g_system_state.buffer_index = 0;
    
    // 初始化调试模式
    // g_system_state.debug_level = 1;
    g_system_state.debug_enabled = false;

    // 设置脉冲完成回调
    StepperMotor_SetPulseCompleteCallback(MotorPulseCompleteCallback);
    
    // 初始化运动状态
    g_system_state.motor_moving = false;
    g_system_state.target_steps = 0;
    g_system_state.current_steps = 0;
    g_system_state.current_direction = '0'; // 0表示停止

    // 初始化INA236通讯状态
    g_system_state.ina236_init_stat = false;
    g_system_state.ina236_read_stat = false;
}

void SystemState_UpdateCurrent(uint16_t current) {
    // 更新电流缓冲区
    g_system_state.current_buffer[g_system_state.buffer_index] = current;
    g_system_state.buffer_index = (g_system_state.buffer_index + 1) % BUFFER_SIZE;
}

void SystemState_ResetRoundCount(void) {
    g_system_state.round_count = 0;
}

void SystemState_UpdateInputs(void) {
    // 这里需要读取GPIO输入状态
    // 实际实现需要根据具体的GPIO配置来修改
    // g_system_state.zero_point = HAL_GPIO_ReadPin(INPUT_ZERO_GPIO_Port, INPUT_ZERO_Pin);
    
    // 检测原点信号上升沿
    static bool last_roundout_state = false;
    bool current_roundout_state = false; // 从GPIO读取
    if (current_roundout_state && !last_roundout_state) {
        g_system_state.round_count++;
    }
    last_roundout_state = current_roundout_state;
}

void SystemState_SaveToEEPROM(void) {
    EE_WriteVariable(0x0001, g_system_state.freq);
    EE_WriteVariable(0x0002, g_system_state.threshold);
    EE_WriteVariable(0x0003, g_system_state.debug_level);
}

void SystemState_LoadFromEEPROM(void) {
    uint32_t freq_value;
    uint32_t threshold_value;
    uint32_t debug_level_value;
    
    if (EE_ReadVariable(0x0001, &freq_value) == 0) {
        g_system_state.freq = (uint16_t)freq_value;
    }
    
    if (EE_ReadVariable(0x0002, &threshold_value) == 0) {
        g_system_state.threshold = (uint16_t)&threshold_value;
    }

    if (EE_ReadVariable(0x0003, &debug_level_value) == 0) {
        g_system_state.debug_level = (uint8_t)debug_level_value;
    }
}