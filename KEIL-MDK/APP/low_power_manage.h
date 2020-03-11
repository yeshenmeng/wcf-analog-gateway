#ifndef __LOW_POWER_MANAGE_H__
#define __LOW_POWER_MANAGE_H__
#include "main.h"
#include "sw_timer_rtc.h"


#define RTC_STATUS_GET() 					(NRF_RTC1->INTENSET != 0)

#define LPM_TASK_STA_INVALID				0
#define LPM_TASK_STA_STOP					1
#define LPM_TASK_STA_RUN					2
#define LPM_TASK_STA_LP						3

#define IS_LPM_TASK_STATE(status) 			(((status) == LPM_TASK_STA_STOP) || \
											((status) == LPM_TASK_STA_RUN)  || \
											((status) == LPM_TASK_STA_LP))

#define LPM_TASK_NULL						4
#define LPM_TASK_REGISTER					5

#define LPM_TASK_MAX_NUMBER					10

typedef void (*lpm_handler_t)(void);

typedef struct {
	uint8_t TaskId;
	uint8_t TaskStatus;
}LPM_Task_t;

typedef enum {
	RTC_STOP = 0,
	RTC_RUN = 1,
}Rtc_Status;

typedef struct
{
	ErrorStatus (*TaskRegister)(uint8_t TaskId);
	ErrorStatus (*TaskCancelRegister)(uint8_t TaskId);
	ErrorStatus (*TaskSetStatus)(uint8_t TaskId, uint8_t state);
	uint8_t (*TaskQueryStatus)(uint8_t TaskId);
	FlagStatus (*IsTaskRun)(void);
	
	void (*LowPowerManage)(lpm_handler_t enter, lpm_handler_t exit);
}LPM_t;

LPM_t* LPM_Init(void);

#endif




