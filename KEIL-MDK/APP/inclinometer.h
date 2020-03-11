#ifndef __INCLINOMETER_H__
#define __INCLINOMETER_H__
#include "main.h"
#include "low_power_manage.h"


#define INCLINOMETER_TASK_ID						4
#define INCLINOMETER_PWOER_PIN					3
#define INCLINOMETER_PWOER_PORT					P0

typedef enum {
	SCA_D01,
	SCA_D02,
	SCA_D07,
}SCA_Type;

typedef enum {
	INCLINOMETER_TASK_IDLE,
	INCLINOMETER_TASK_ACTIVE,
	INCLINOMETER_TASK_STOP,
}Inclinometer_Task_State;

typedef struct {
	uint8_t UpdateFlag;
	float Temperature;
	float XAngle;
	float YAngle;
}Inclinometer_Data;

typedef struct {
	Inclinometer_Task_State State;
	Inclinometer_Data Data;
	LPM_t* LPMHandle;
	
	void (*TaskStart)(void);
	void (*TaskStop)(void);
	void (*TaskOperate)(void);
	float (*ReadTemp)(void);
	float (*ReadXAngle)(uint8_t ReadTimes);
	float (*ReadYAngle)(uint8_t ReadTimes);
}Inclinometer_Info_t;

Inclinometer_Info_t* Inclinometer_TaskInit(LPM_t* LPMHandle);


#endif


