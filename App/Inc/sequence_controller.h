#ifndef __SEQUENCE_CONTROLLER_H__
#define __SEQUENCE_CONTROLLER_H__

#include "stdbool.h"

// 序列状态
typedef enum {
    SEQ_IDLE = 0,
    SEQ_SETUP_SWITCHES,
    SEQ_START_MOVING,
    SEQ_MONITOR_CURRENT,
    SEQ_ADJUST_SWITCHES,
    SEQ_FINAL_MOVE,
    SEQ_COMPLETE
} SequenceState_t;

// 函数声明
void SequenceController_Init(void);
void SequenceController_Start(void);
void SequenceController_Process(void);
bool SequenceController_IsRunning(void);
SequenceState_t SequenceController_GetState(void);

#endif /* __SEQUENCE_CONTROLLER_H__ */