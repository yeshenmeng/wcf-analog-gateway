#ifndef __CMD_DEBUG_H__
#define __CMD_DEBUG_H__
#include "main.h"


void cmd_data_rx(uint8_t* data, uint8_t size);
void cmd_data_parse(void);
void cmd_data_process(void);
void cmd_data_reply(void);


#endif

