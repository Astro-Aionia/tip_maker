#ifndef __STEPPER_MOTOR_H__
#define __STEPPER_MOTOR_H__

#include "stdint.h"
#include "stdbool.h"

// 电机方向定义
typedef enum {
    MOTOR_DIR_CW = 0,
    MOTOR_DIR_CCW = 1,
    MOTOR_DIR_STOP = 2
} MotorDirection_t;

// 函数声明
void StepperMotor_Init(void);
void StepperMotor_SetFrequency(uint16_t freq);
void StepperMotor_Move(MotorDirection_t dir, uint16_t steps);
void StepperMotor_Stop(void);
bool StepperMotor_IsMoving(void);
void StepperMotor_UpdateSwitches(bool holdoff, bool division);
void StepperMotor_Process(void);

#endif /* __STEPPER_MOTOR_H__ */