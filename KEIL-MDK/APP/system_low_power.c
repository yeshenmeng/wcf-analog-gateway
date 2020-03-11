#include "system_low_power.h"
#include "sw_timer_rtc.h"
#include "sys_param.h"
#include "calendar.h"
#include "light.h"


static SLP_State SlpState;
static SLP_Info_t SLP_Info;

__weak void SLP_TaskStopHandler(void* param);


static void SLP_TaskStart(void)
{
	if(SLP_Info.State == SLP_IDLE)
	{
		SLP_Info.LPMHandle->TaskSetStatus(SLP_TASK_ID, LPM_TASK_STA_RUN);
		SlpState = SLP_ACTIVE;
		SLP_Info.State = SLP_ACTIVE;
	}
	return;
}

static void SLP_TaskStop(void)
{
	if(SLP_Info.State != SLP_IDLE)
	{
		SlpState = SLP_STOP;
	}
	return;	
}

void SWT_SysLowPowerCallback(void)
{
	SlpState = SLP_TIMEOUT;
	return;	
}

static void SLP_StatusProc(void)
{
	SLP_State stateTmp;
	SWT_t* timer = SWT_GetHandle();
	
	switch((uint8_t)SlpState)
	{
		case SLP_ACTIVE:
			{
				sys_param_t* param = Sys_ParamGetHandle();
				Calendar_t* calendar = Calendar_GetHandle();
				SlpState = SLP_IDLE;
				SLP_Info.State = SLP_ACTIVE;
				uint32_t timeStamp = calendar->GetTimeStamp();
				uint32_t sampleInterval = param->iot_sample_interval;
				SLP_Info.SleepTime = sampleInterval - timeStamp % sampleInterval;
				timer->SysLowPower->Start(SLP_Info.SleepTime*1000);
				SLP_Info.LPMHandle->TaskSetStatus(SLP_TASK_ID, LPM_TASK_STA_LP);
			}
			break;
		
		/* 系统低功耗睡眠定时器超时事件 */
		case SLP_TIMEOUT:
		/* 主动停止系统睡眠活动事件 */
		case SLP_STOP:
			stateTmp = SlpState;
			SlpState = SLP_IDLE;
			timer->SysLowPower->Stop();
			SLP_Info.State = SLP_IDLE;
			SLP_Info.LPMHandle->TaskSetStatus(SLP_TASK_ID, LPM_TASK_STA_STOP);
			SLP_TaskStopHandler((void *)&stateTmp); /* 系统低功耗任务停止处理 */
			break;
	}
	
	return;
}

SLP_Info_t* SLP_TaskInit(LPM_t* LPMHandle)
{
	SlpState = SLP_IDLE;
	SLP_Info.State = SLP_IDLE;	
	
	SLP_Info.SleepTime = 3000;
	
	SLP_Info.LPMHandle = LPMHandle;
	
	SLP_Info.TaskStart = SLP_TaskStart;
	SLP_Info.TaskStop = SLP_TaskStop;
	SLP_Info.StatusProc = SLP_StatusProc;
	
	SLP_Info.LPMHandle->TaskRegister(SLP_TASK_ID);
	
	return &SLP_Info;
}

/* 系统低功耗任务停止处理 */
__weak void SLP_TaskStopHandler(void* param)
{
	return;
}







