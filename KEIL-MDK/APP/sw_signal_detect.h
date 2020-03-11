#ifndef __SW_SIGNAL_DETECT_H__
#define __SW_SIGNAL_DETECT_H__
#include "main.h"
#include "low_power_manage.h"


#define SIGNAL_DET_TASK_ID							5
#define SIGNAL_DET_TIME_SLICE						10*1000u /* –≈∫≈ºÏ≤‚ ±º‰∆¨ */


typedef enum {
	SIGNAL_DET_IDLE,
	SIGNAL_DET_ACTIVE,
	SIGNAL_DET_TIMEOUT,
	SIGNAL_DET_STOP,
}Signal_Det_State;

typedef struct {
	uint32_t ActiveTime;
	Signal_Det_State State;
	
	LPM_t* LPMHandle;
	
	void (*TaskStart)(void);
	void (*TaskStop)(void);
	void (*StatusProc)(void);
}Signal_Det_Info_t;


Signal_Det_Info_t* Signal_DetTaskInit(LPM_t* LPMHandle);


#endif


