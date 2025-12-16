#include "stepper_motor.h"
#include "system_state.h"
#include "hal_instances.h"
#include <string.h>

// 内部状态
static struct {
    MotorDirection_t direction;
    uint16_t target_pulses;
    uint16_t current_pulses;
    bool is_moving;
} motor_state;

void StepperMotor_Init(void) {
    motor_state.direction = MOTOR_DIR_STOP;
    motor_state.target_pulses = 0;
    motor_state.current_pulses = 0;
    motor_state.is_moving = false;
    
    // 停止PWM输出，设置引脚为高电平
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    
    // 设置引脚为输出模式并置高
    HAL_GPIO_WritePin(PWM_CW_GPIO_Port, PWM_CW_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_CCW_GPIO_Port, PWM_CCW_Pin, GPIO_PIN_SET);
}

void StepperMotor_SetFrequency(uint16_t freq) {
    if (freq == 0) {
        // 频率为0时停止电机
        StepperMotor_Stop();
        return;
    }
    
    if (freq > 1000) freq = 1000;
    
    // 计算ARR值：ARR = (时钟频率 / 预分频) / 目标频率 - 1
    // 假设TIM1时钟为72MHz，预分频为72，得到1MHz计数器时钟
    uint32_t arr = 1000000 / freq - 1;
    
    __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, arr / 2);  // 50%占空比
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, arr / 2);
}

void StepperMotor_Move(MotorDirection_t dir, uint16_t steps) {
    if (steps == 0 || dir == MOTOR_DIR_STOP) {
        StepperMotor_Stop();
        return;
    }
    
    // 设置目标步数
    motor_state.target_pulses = steps;
    motor_state.current_pulses = 0;
    motor_state.direction = dir;
    motor_state.is_moving = true;
    
    // 设置频率为固有频率
    StepperMotor_SetFrequency(g_system_state.origin_freq);
    
    // 启动相应方向的PWM
    if (dir == MOTOR_DIR_CW) {
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    } else {
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    }
    
    // 更新系统状态
    g_system_state.motor_moving = true;
    g_system_state.target_steps = steps;
    g_system_state.current_steps = 0;
    g_system_state.current_direction = (dir == MOTOR_DIR_CW) ? 'C' : 'W';
}

void StepperMotor_Stop(void) {
    motor_state.is_moving = false;
    motor_state.direction = MOTOR_DIR_STOP;
    
    // 停止PWM，设置引脚为高电平
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    
    HAL_GPIO_WritePin(PWM_CW_GPIO_Port, PWM_CW_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_CCW_GPIO_Port, PWM_CCW_Pin, GPIO_PIN_SET);
    
    // 更新系统状态
    g_system_state.motor_moving = false;
    g_system_state.current_direction = 'S';
}

bool StepperMotor_IsMoving(void) {
    return motor_state.is_moving;
}

void StepperMotor_UpdateSwitches(bool holdoff, bool division) {
    // 更新HOLDOFF开关（注意：ON代表False，OFF代表True）
    HAL_GPIO_WritePin(SWITCH_HOLDOFF_GPIO_Port, SWITCH_HOLDOFF_Pin, 
                     holdoff ? GPIO_PIN_RESET : GPIO_PIN_SET);
    
    // 更新细分选择开关
    HAL_GPIO_WritePin(SWITCH_DIVISION_GPIO_Port, SWITCH_DIVISION_Pin, 
                     division ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void StepperMotor_Process(void) {
    if (!motor_state.is_moving) return;
    
    // 检查是否到达目标步数
    // 这里可以通过定时器中断或编码器输入来更新current_pulses
    // 示例中使用简单的计数方式
    if (motor_state.current_pulses >= motor_state.target_pulses) {
        StepperMotor_Stop();
    }
}