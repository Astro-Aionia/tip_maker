#include "command_parser.h"
#include "system_state.h"
#include "stepper_motor.h"
#include "sequence_controller.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// 命令缓冲区
static char cmd_buffer[MAX_CMD_LENGTH];
static uint8_t cmd_index = 0;

void CommandParser_Init(void) {
    cmd_index = 0;
    memset(cmd_buffer, 0, sizeof(cmd_buffer));
}

void CommandParser_USBReceiveCallback(uint8_t *buf, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        if (buf[i] == '\n' || buf[i] == '\r') {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                CommandParser_Process(cmd_buffer);
                cmd_index = 0;
            }
        } else if (cmd_index < MAX_CMD_LENGTH - 1) {
            cmd_buffer[cmd_index++] = buf[i];
        }
    }
}

void CommandParser_Process(const char *cmd) {
    char response[256] = {0};
    
    // 解析命令类型
    if (strncmp(cmd, "SET ", 4) == 0) {
        // SET命令处理
        char key[32] = {0};
        char value[32] = {0};
        
        if (sscanf(cmd + 4, "%s %s", key, value) == 2) {
            bool success = true;
            
            if (strcmp(key, "FREQ") == 0) {
                uint16_t freq = atoi(value);
                if (freq >= 0 && freq <= 1000) {
                    g_system_state.freq = freq;
                    // SystemState_SaveToEEPROM();
                } else {
                    success = false;
                }
            } 
            else if (strcmp(key, "THRES") == 0) {
                int16_t thres = atoi(value);
                g_system_state.threshold = thres;
                // SystemState_SaveToEEPROM();
            }
            else if (strcmp(key, "CURRENT") == 0) {
                if (strcmp(value, "ON") == 0) {
                    g_system_state.switch_current = true;
                    HAL_GPIO_WritePin(SWITCH_CURRENT_GPIO_Port, SWITCH_CURRENT_Pin, GPIO_PIN_SET);
                } else if (strcmp(value, "OFF") == 0) {
                    g_system_state.switch_current = false;
                    HAL_GPIO_WritePin(SWITCH_CURRENT_GPIO_Port, SWITCH_CURRENT_Pin, GPIO_PIN_RESET);
                } else {
                    success = false;
                }
            }
            else if (strcmp(key, "HOLDOFF") == 0) {
                if (strcmp(value, "ON") == 0) {
                    g_system_state.switch_holdoff = false;  // ON代表False
                    StepperMotor_UpdateSwitches(false, g_system_state.switch_division);
                } else if (strcmp(value, "OFF") == 0) {
                    g_system_state.switch_holdoff = true;   // OFF代表True
                    StepperMotor_UpdateSwitches(true, g_system_state.switch_division);
                } else {
                    success = false;
                }
            }
            else if (strcmp(key, "DIVISION") == 0) {
                if (strcmp(value, "ON") == 0) {
                    g_system_state.switch_division = true;
                    StepperMotor_UpdateSwitches(g_system_state.switch_holdoff, true);
                } else if (strcmp(value, "OFF") == 0) {
                    g_system_state.switch_division = false;
                    StepperMotor_UpdateSwitches(g_system_state.switch_holdoff, false);
                } else {
                    success = false;
                }
            }
            else {
                success = false;
            }
            
            // 发送响应
            if (success) {
                snprintf(response, sizeof(response), 
                        "{\"Cmd\": \"SET\", \"Status\": \"Success\", \"Parameter\": \"%s\", \"Value\": \"%s\"}\r\n",
                        key, value);
            } else {
                snprintf(response, sizeof(response), 
                        "{\"Cmd\": \"SET\", \"Status\": \"Error\", \"Parameter\": \"%s\", \"Value\": \"%s\"}\r\n",
                        key, value);
            }
        }
    }
    else if (strncmp(cmd, "GET ", 4) == 0) {
        // GET命令处理
        char key[32] = {0};
        sscanf(cmd + 4, "%s", key);
        
        char value_str[32] = {0};
        bool success = true;
        
        if (strcmp(key, "FREQ") == 0) {
            snprintf(value_str, sizeof(value_str), "%d", g_system_state.freq);
        }
        else if (strcmp(key, "THRES") == 0) {
            snprintf(value_str, sizeof(value_str), "%d", g_system_state.threshold);
        }
        else if (strcmp(key, "CURRENT") == 0) {
            snprintf(value_str, sizeof(value_str), "%s", 
                    g_system_state.switch_current ? "ON" : "OFF");
        }
        else if (strcmp(key, "HOLDOFF") == 0) {
            snprintf(value_str, sizeof(value_str), "%s", 
                    g_system_state.switch_holdoff ? "OFF" : "ON");
        }
        else if (strcmp(key, "DIVISION") == 0) {
            snprintf(value_str, sizeof(value_str), "%s", 
                    g_system_state.switch_division ? "ON" : "OFF");
        }
        else {
            success = false;
        }
        
        if (success) {
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"GET\", \"Status\": \"Success\", \"Parameter\": \"%s\", \"Value\": \"%s\"}\r\n",
                    key, value_str);
        } else {
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"GET\", \"Status\": \"Error\", \"Parameter\": \"%s\"}\r\n",
                    key);
        }
    }
    else if (strncmp(cmd, "MOVE ", 5) == 0) {
        // MOVE命令处理
        char dir[8] = {0};
        int steps = 0;
        
        if (sscanf(cmd + 5, "%s %d", dir, &steps) == 2) {
            if (strcmp(dir, "CW") == 0) {
                StepperMotor_Move(MOTOR_DIR_CW, steps);
                snprintf(response, sizeof(response), 
                        "{\"Cmd\": \"MOVE\", \"Status\": \"Success\", \"Direction\": \"CW\", \"Step\": %d}\r\n",
                        steps);
            }
            else if (strcmp(dir, "CCW") == 0) {
                StepperMotor_Move(MOTOR_DIR_CCW, steps);
                snprintf(response, sizeof(response), 
                        "{\"Cmd\": \"MOVE\", \"Status\": \"Success\", \"Direction\": \"CCW\", \"Step\": %d}\r\n",
                        steps);
            }
            else {
                snprintf(response, sizeof(response), 
                        "{\"Cmd\": \"MOVE\", \"Status\": \"Error\", \"Direction\": \"%s\", \"Step\": %d}\r\n",
                        dir, steps);
            }
        } else {
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"MOVE\", \"Status\": \"Error\"}\r\n");
        }
    }
    else if (strcmp(cmd, "POLL") == 0) {
        // POLL命令处理
        snprintf(response, sizeof(response), 
                "{\"Cmd\": \"POLL\", \"Status\": \"Success\", \"Data\": [");
        
        for (int i = 0; i < 8; i++) {
            char temp[16];
            snprintf(temp, sizeof(temp), "%.6f", g_system_state.current_buffer[i]);
            strcat(response, temp);
            
            if (i < 7) {
                strcat(response, ", ");
            }
        }
        strcat(response, "]}\r\n");
    }
    else if (strcmp(cmd, "START") == 0) {
        // START命令处理
        SequenceController_Start();
        snprintf(response, sizeof(response), 
                "{\"Cmd\": \"START\", \"Status\": \"Success\"}\r\n");
    }
    else if (strncmp(cmd, "STATUS ", 7) == 0) {
        // STATUS命令处理
        int level = atoi(cmd + 7);
        
        if (level >= 1 && level <= 3) {
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"STATUS\", \"Status\": \"Success\", \"Level\": %d", level);
            
            // Level 1: 用户可设置的5个参数
            char temp[128];
            snprintf(temp, sizeof(temp), 
                    ", \"FREQ\": %d, \"THRES\": %.6f, \"CURRENT\": \"%s\", "
                    "\"HOLDOFF\": \"%s\", \"DIVISION\": \"%s\"",
                    g_system_state.freq, g_system_state.threshold,
                    g_system_state.switch_current ? "ON" : "OFF",
                    g_system_state.switch_holdoff ? "OFF" : "ON",
                    g_system_state.switch_division ? "ON" : "OFF");
            strcat(response, temp);
            
            // Level 2: 添加最后一个电流值
            if (level >= 2) {
                snprintf(temp, sizeof(temp), 
                        ", \"LastCurrent\": %.6f", 
                        g_system_state.current_buffer[(g_system_state.buffer_index + 7) % 8]);
                strcat(response, temp);
            }
            
            // Level 3: 添加详细状态
            if (level >= 3) {
                snprintf(temp, sizeof(temp), 
                        ", \"MotorMoving\": %s, \"TargetSteps\": %d, \"CurrentSteps\": %d, "
                        "\"CurrentDirection\": \"%c\"",
                        g_system_state.motor_moving ? "true" : "false",
                        g_system_state.target_steps, g_system_state.current_steps,
                        g_system_state.current_direction
                        /*", \"MotorMoving\": %s, \"TargetSteps\": %d, \"CurrentSteps\": %d, "
                        "\"CurrentDirection\": \"%c\", \"RoundCount\": %d, \"ZeroPoint\": %s",
                        g_system_state.motor_moving ? "true" : "false",
                        g_system_state.target_steps, g_system_state.current_steps,
                        g_system_state.current_direction,
                        g_system_state.round_count,
                        g_system_state.zero_point ? "true" : "false"*/);
                strcat(response, temp);
            }
            
            strcat(response, "}\r\n");
        } else {
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"STATUS\", \"Status\": \"Error\", \"Level\": %d}\r\n", level);
        }
    }
    else if (strncmp(cmd, "DEBUG ", 6) == 0) {
        // DEBUG命令处理
        int level = atoi(cmd + 6);
        
        if (level == 0) {
            g_system_state.debug_enabled = false;
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"DEBUG\", \"Status\": \"Success\", \"DebugMode\": \"OFF\"}\r\n");
        }
        else if (level >= 1 && level <= 3) {
            g_system_state.debug_enabled = true;
            g_system_state.debug_level = level;
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"DEBUG\", \"Status\": \"Success\", \"DebugMode\": \"ON\", \"Level\": %d}\r\n",
                    level);
        } else {
            snprintf(response, sizeof(response), 
                    "{\"Cmd\": \"DEBUG\", \"Status\": \"Error\", \"Level\": %d}\r\n", level);
        }
    }
    else {
        // 未知命令
        snprintf(response, sizeof(response), 
                "{\"Cmd\": \"UNKNOWN\", \"Status\": \"Error\", \"Message\": \"Unknown command\"}\r\n");
    }
    
    // 发送响应
    CDC_Transmit_FS((uint8_t*)response, strlen(response));
}