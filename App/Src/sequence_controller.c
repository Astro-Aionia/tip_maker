#include "sequence_controller.h"
#include "system_state.h"
#include "stepper_motor.h"
#include "gpio.h"
#include <stdbool.h>

static SequenceState_t seq_state = SEQ_IDLE;
static uint32_t seq_timer = 0;
static bool seq_running = false;

void SequenceController_Init(void) {
    seq_state = SEQ_IDLE;
    seq_running = false;
}

void SequenceController_Start(void) {
    if (!seq_running) {
        seq_state = SEQ_SETUP_SWITCHES;
        seq_running = true;
        seq_timer = HAL_GetTick();
    }
}

bool SequenceController_IsRunning(void) {
    return seq_running;
}

void SequenceController_Process(void) {
    if (!seq_running) return;
    
    uint32_t current_time = HAL_GetTick();
    
    switch (seq_state) {
        case SEQ_SETUP_SWITCHES:
            // 设置开关状态
            g_system_state.switch_current = true;
            g_system_state.switch_holdoff = false;
            g_system_state.switch_division = true;
            
            // 应用到GPIO
            HAL_GPIO_WritePin(SWITCH_CURRENT_GPIO_Port, SWITCH_CURRENT_Pin, GPIO_PIN_SET);
            StepperMotor_UpdateSwitches(false, true);
            
            seq_state = SEQ_START_MOVING;
            seq_timer = current_time;
            break;
            
        case SEQ_START_MOVING:
            // 等待开关稳定（如果需要）
            if (current_time - seq_timer > 100) {
                // 以可调频率启动CW方向连续运动
                StepperMotor_SetFrequency(g_system_state.freq);
                StepperMotor_Move(MOTOR_DIR_CW, 0xFFFF); // 最大步数，表示连续运动
                
                seq_state = SEQ_MONITOR_CURRENT;
                seq_timer = current_time;
            }
            break;
            
        case SEQ_MONITOR_CURRENT:
            // 检查所有电流值是否低于阈值
            bool all_below = true;
            for (int i = 0; i < 8; i++) {
                if (g_system_state.current_buffer[i] >= g_system_state.threshold) {
                    all_below = false;
                    break;
                }
            }
            
            if (all_below) {
                seq_state = SEQ_ADJUST_SWITCHES;
                seq_timer = current_time;
            }
            break;
            
        case SEQ_ADJUST_SWITCHES:
            // 调整开关状态
            g_system_state.switch_current = false;
            g_system_state.switch_division = false;
            
            // 应用到GPIO
            HAL_GPIO_WritePin(SWITCH_CURRENT_GPIO_Port, SWITCH_CURRENT_Pin, GPIO_PIN_RESET);
            StepperMotor_UpdateSwitches(g_system_state.switch_holdoff, false);
            
            seq_state = SEQ_FINAL_MOVE;
            seq_timer = current_time;
            break;
            
        case SEQ_FINAL_MOVE:
            // 停止当前运动，以固有频率运动2000步
            StepperMotor_Stop();
            
            // 等待停止
            if (current_time - seq_timer > 50) {
                StepperMotor_SetFrequency(g_system_state.origin_freq);
                StepperMotor_Move(MOTOR_DIR_CW, 2000);
                
                seq_state = SEQ_COMPLETE;
            }
            break;
            
        case SEQ_COMPLETE:
            // 检查电机是否停止
            if (!StepperMotor_IsMoving()) {
                seq_running = false;
                seq_state = SEQ_IDLE;
            }
            break;
            
        case SEQ_IDLE:
        default:
            seq_running = false;
            break;
    }
}