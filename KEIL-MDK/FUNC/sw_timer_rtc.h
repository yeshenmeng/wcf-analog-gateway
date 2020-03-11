#ifndef __SW_TIMER_RTC_H__
#define __SW_TIMER_RTC_H__
#include "main.h"
#include "app_timer.h"


typedef struct
{
	uint8_t TimerID;
	uint32_t ProcessID;
	app_timer_mode_t Mode;
	app_timer_timeout_handler_t Callback;
	
	void (*Start)(uint32_t time); /* time:基于RTC唤醒定时器tick(CFG_TS_TICK_VAL:500us)的定时时间500us-32.7675s 
																	 取决于异步与同步分配值设置*/
	void (*Stop)(void);
	void (*Delete)(void);
}SWTimer_t;

typedef struct {
	SWTimer_t* SysIdle;
	SWTimer_t* SysLowPower;
	SWTimer_t* BleAdv;
	SWTimer_t* BleAdvLed;
	SWTimer_t* LoraTaskTimeSlice;
	SWTimer_t* LoraTxTimeout;
	SWTimer_t* LoraIdle;
	SWTimer_t* SignalDetTimeSlice;
}SWT_t;

void SWT_Init(void);
SWT_t* SWT_GetHandle(void);
SWTimer_t* SWT_SysLpCreate(void);
SWTimer_t* SWT_BleAdvCreate(void);
SWTimer_t* SWT_LoraLpCreate(void);
SWTimer_t* SWT_LoraTxTimeoutCreate(void);
SWTimer_t* SWT_LoraIdleCreate(void);


#endif



