#ifndef __COMMAND_PARSER_H__
#define __COMMAND_PARSER_H__

#include "stdint.h"
#include "stdbool.h"

// 最大命令长度
#define MAX_CMD_LENGTH 64

// 函数声明
void CommandParser_Init(void);
void CommandParser_Process(const char *cmd);
void CommandParser_USBReceiveCallback(uint8_t *buf, uint32_t len);

#endif /* __COMMAND_PARSER_H__ */