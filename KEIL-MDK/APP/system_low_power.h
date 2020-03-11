#ifndef __SYSTEM_LOW_POWER_H__
#define __SYSTEM_LOW_POWER_H__
#include "main.h"
#include "low_power_manage.h"


#define SLP_TASK_ID								3

typedef enum {
	SLP_IDLE,
	SLP_ACTIVE,
	SLP_TIMEOUT,
	SLP_STOP,
}SLP_State;

typedef struct {
	uint32_t SleepTime;
	SLP_State State;
	
	LPM_t* LPMHandle;
	
	void (*TaskStart)(void);
	void (*TaskStop)(void);
	void (*StatusProc)(void);
}SLP_Info_t;

SLP_Info_t* SLP_TaskInit(LPM_t* LPMHandle);

#endif


