#ifndef __STEPPER_MOTOR_H__
#define __STEPPER_MOTOR_H__

#include "stdint.h"
#include "stdbool.h"

// 电机方向定义
typedef enum {
    MOTOR_DIR_STOP = 0,
    MOTOR_DIR_CW = 1,
    MOTOR_DIR_CCW = 2
} MotorDirection_t;

// 脉冲计数回调函数类型
typedef void (*PulseCompleteCallback_t)(void);

// 函数声明
void StepperMotor_Init(void);
void StepperMotor_SetFrequency(uint16_t freq);
void StepperMotor_Move(MotorDirection_t dir, uint16_t steps);
void StepperMotor_CountinueMove(MotorDirection_t dir);
void StepperMotor_Stop(void);
bool StepperMotor_IsMoving(void);
void StepperMotor_UpdateSwitches(bool holdoff, bool division);
void StepperMotor_Process(void);
void StepperMotor_SetPulseCompleteCallback(PulseCompleteCallback_t callback);
uint16_t StepperMotor_GetCurrentPulses(void);
void StepperMotor_ResetPulseCount(void);

// TIM1中断处理函数
void StepperMotor_TIM1_Update_IRQHandler(void);

#endif /* __STEPPER_MOTOR_H__ */