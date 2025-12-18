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
    bool counting_enabled;
    PulseCompleteCallback_t pulse_complete_callback;
} motor_state;

// 用于存储TIM1的ARR和PSC值
static uint16_t tim1_arr_value = 999; // 默认1000Hz: (1MHz/1000) - 1
static uint16_t tim1_psc_value = 71;  // 72MHz/72 = 1MHz

void StepperMotor_Init(void) {
    motor_state.direction = MOTOR_DIR_STOP;
    motor_state.target_pulses = 0;
    motor_state.current_pulses = 0;
    motor_state.is_moving = false;
    motor_state.counting_enabled = false;
    motor_state.pulse_complete_callback = NULL;

    // 获取TIM1的配置值
    tim1_arr_value = htim1.Instance->ARR;
    tim1_psc_value = htim1.Instance->PSC;
    
    // 停止PWM输出，设置引脚为高电平
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_Base_Stop_IT(&htim1);
    
    // 设置引脚为输出模式并置高
    HAL_GPIO_WritePin(PWM_CW_GPIO_Port, PWM_CW_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_CCW_GPIO_Port, PWM_CCW_Pin, GPIO_PIN_SET);

    // 清除更新中断标志
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
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
    tim1_arr_value = arr;

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
    motor_state.counting_enabled = true;
    
    // 设置频率为固有频率
    StepperMotor_SetFrequency(ORIGIN_FREQ);

    // 重置计数器
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    
    // 清除所有中断标志
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC1);
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC2);
    
    // 启动相应方向的PWM
    if (dir == MOTOR_DIR_CW) {
        // 停止另一个方向
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_GPIO_WritePin(PWM_CCW_GPIO_Port, PWM_CCW_Pin, GPIO_PIN_SET);
        
        // 启动CW方向PWM和定时器中断
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_Base_Start_IT(&htim1);
    } else if (dir == MOTOR_DIR_CCW) {
        // 停止另一个方向
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_GPIO_WritePin(PWM_CW_GPIO_Port, PWM_CW_Pin, GPIO_PIN_SET);
        
        // 启动CCW方向PWM和定时器中断
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_Base_Start_IT(&htim1);
    } else {
        // 无效方向，停止电机
        StepperMotor_Stop();
        return;
    }
    
    // 更新系统状态
    g_system_state.motor_moving = true;
    g_system_state.target_steps = steps;
    g_system_state.current_steps = 0;
    g_system_state.current_direction = (dir == MOTOR_DIR_CW) ? '+' : '-';
}

void StepperMotor_CountinueMove(MotorDirection_t dir){
    // 使用最大步数表示连续运动，禁用脉冲计数
    motor_state.counting_enabled = false;
    motor_state.target_pulses = 0xFFFF; // 最大值表示连续运动
    motor_state.current_pulses = 0;
    motor_state.direction = dir;
    motor_state.is_moving = true;

    if (motor_state.direction == MOTOR_DIR_CW) {
        // 停止另一个方向
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_GPIO_WritePin(PWM_CCW_GPIO_Port, PWM_CCW_Pin, GPIO_PIN_SET);
        
        // 启动CW方向PWM并关闭定时器中断
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_Base_Stop_IT(&htim1);
    } else if (motor_state.direction == MOTOR_DIR_CCW) {
        // 停止另一个方向
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_GPIO_WritePin(PWM_CW_GPIO_Port, PWM_CW_Pin, GPIO_PIN_SET);
        
        // 启动CCW方向PWM并关闭定时器中断
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_Base_Stop_IT(&htim1);
    } else {
        // 无效方向，停止电机
        StepperMotor_Stop();
        return;
    }
}

void StepperMotor_Stop(void) {
    motor_state.is_moving = false;
    motor_state.direction = MOTOR_DIR_STOP;
    motor_state.counting_enabled = false;

    // 停止PWM和定时器中断
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_Base_Stop_IT(&htim1);
    
    // 停止PWM，设置引脚为高电平    
    HAL_GPIO_WritePin(PWM_CW_GPIO_Port, PWM_CW_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWM_CCW_GPIO_Port, PWM_CCW_Pin, GPIO_PIN_SET);

    // 清除中断标志
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
    
    // 更新系统状态
    g_system_state.motor_moving = false;
    g_system_state.current_direction = '0';
    g_system_state.current_steps = 0;
    g_system_state.target_steps = 0;

     // 如果有回调函数，调用它
    if (motor_state.pulse_complete_callback != NULL) {
        motor_state.pulse_complete_callback();
    }
}

bool StepperMotor_IsMoving(void) {
    return motor_state.is_moving;
}

void StepperMotor_UpdateSwitches(bool holdoff, bool division) {
    // 更新HOLDOFF开关
    HAL_GPIO_WritePin(SWITCH_HOLDOFF_GPIO_Port, SWITCH_HOLDOFF_Pin, 
                     holdoff ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // 更新细分选择开关
    HAL_GPIO_WritePin(SWITCH_DIVISION_GPIO_Port, SWITCH_DIVISION_Pin, 
                     division ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void StepperMotor_Process(void) {
    if (!motor_state.is_moving) return;
    
    // 更新系统状态中的当前步数
    g_system_state.current_steps = motor_state.current_pulses;
    
    // 检查是否到达目标步数
    if (motor_state.counting_enabled && 
        motor_state.current_pulses >= motor_state.target_pulses) {
        StepperMotor_Stop();
    }
}

void StepperMotor_SetPulseCompleteCallback(PulseCompleteCallback_t callback) {
    motor_state.pulse_complete_callback = callback;
}

uint16_t StepperMotor_GetCurrentPulses(void) {
    return motor_state.current_pulses;
}

void StepperMotor_ResetPulseCount(void) {
    motor_state.current_pulses = 0;
}

// TIM1更新中断处理函数（用于脉冲计数）
void StepperMotor_TIM1_Update_IRQHandler(void) {
    // 检查是否更新中断
    if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) != RESET) {
        if (__HAL_TIM_GET_IT_SOURCE(&htim1, TIM_IT_UPDATE) != RESET) {
            // 清除更新中断标志
            __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);
            
            // 如果计数使能，增加脉冲计数
            if (motor_state.counting_enabled) {
                motor_state.current_pulses++;
                
                // 调试输出（可选）
                // printf("Pulse count: %d/%d\n", motor_state.current_pulses, motor_state.target_pulses);
            }
        }
    }
}

// 外部中断处理函数（用于原点励磁计数）
void StepperMotor_EXTI3_Update_IRQHandler(void) {
     // 检查是否是PA3触发的中断
    if (__HAL_GPIO_EXTI_GET_IT(INPUT_ROUNDOUT_Pin) != RESET) {
        // 清除中断标志
        __HAL_GPIO_EXTI_CLEAR_IT(INPUT_ROUNDOUT_Pin);
        
        // 检查电机方向
        switch (g_system_state.current_direction) {
        case 'C': // 正转
            SystemState_UpdateRoundCount(true); // 增加
            break;
            
        case 'W': // 反转
            SystemState_UpdateRoundCount(false); // 减少
            break;
            
        case 'S': // 停止
        default:
            // 停止状态，忽略信号（可能是干扰）
            break;
        }
    }
}