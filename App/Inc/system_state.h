#ifndef __SYSTEM_STATE_H__
#define __SYSTEM_STATE_H__

#include "stdint.h"
#include "stdbool.h"

// 系统固定参数
#define ORIGIN_FREQ 1000 // 默认频率1000Hz
#define BUFFER_SIZE 8   // 电流缓冲区大小

// 系统状态结构体
typedef struct {
    // 固有参数
    // uint16_t origin_freq;
    
    // 用户可调参数
    uint16_t freq;
    int16_t threshold;
    
    // 开关状态
    bool switch_current;
    bool switch_holdoff;
    bool switch_division;
    
    // 输入状态
    bool zero_point;
    uint16_t round_count;
    
    // 电流缓冲区
    int16_t current_buffer[BUFFER_SIZE];
    uint8_t buffer_index;
    
    // 调试模式
    uint8_t debug_level;
    bool debug_enabled;
    
    // 运动状态
    bool motor_moving;
    uint16_t target_steps;
    uint16_t current_steps;
    char current_direction;

    //INA236 通讯状态
    bool ina236_init_stat;
    bool ina236_read_stat;
} SystemState_t;

// 全局系统状态实例
extern SystemState_t g_system_state;

// 函数声明
void SystemState_Init(void);
void SystemState_UpdateCurrent(uint16_t current);
void SystemState_ResetRoundCount(void);
void SystemState_ZeroPoint(void);
void SystemState_SaveToEEPROM(void);
void SystemState_LoadFromEEPROM(void);
void SystemState_UpdateRoundCount(bool increment); // 处理round_count变化
uint16_t SystemState_GetRoundCount(void); // 安全获取round_count（临界区保护）

#endif /* __SYSTEM_STATE_H__ */