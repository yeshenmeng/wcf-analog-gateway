#include "sw_signal_detect.h"
#include "sw_timer_rtc.h"
#include "nrf_drv_gpiote.h"


static Signal_Det_State SignalDetState;
static Signal_Det_Info_t Signal_Det_Info;

__weak void Signal_DetTaskStopHandler(void* param);

static void Signal_DetTaskStart(void)
{
	if(Signal_Det_Info.State == SIGNAL_DET_IDLE)
	{
		Signal_Det_Info.LPMHandle->TaskSetStatus(SIGNAL_DET_TASK_ID, LPM_TASK_STA_RUN);
		SignalDetState = SIGNAL_DET_ACTIVE;
		Signal_Det_Info.State = SIGNAL_DET_ACTIVE;
	}
	return;
}

static void Signal_DetTaskStop(void)
{
	if(Signal_Det_Info.State != SIGNAL_DET_IDLE)
	{
		SignalDetState = SIGNAL_DET_STOP;
	}
	return;	
}

void SWT_SignalDetTimeSliceCallback(void* param)
{
	SignalDetState = SIGNAL_DET_TIMEOUT;
	return;
}

static void Signal_Detect(void)
{
	if(Signal_Det_Info.State == SIGNAL_DET_IDLE)
	{
		return;
	}
	
	SWT_t* timer = SWT_GetHandle();
	switch((uint8_t)SignalDetState)
	{
		case SIGNAL_DET_ACTIVE:
			SignalDetState = SIGNAL_DET_IDLE;
			Signal_Det_Info.State = SIGNAL_DET_ACTIVE;
			timer->SignalDetTimeSlice->Start(Signal_Det_Info.ActiveTime);
			Signal_Det_Info.LPMHandle->TaskSetStatus(SIGNAL_DET_TASK_ID, LPM_TASK_STA_LP);
			break;
		
		/* 信号检测定时器超时事件 */
		case SIGNAL_DET_TIMEOUT:
		/* 主动停止信号检测任务事件 */
		case SIGNAL_DET_STOP:
			SignalDetState = SIGNAL_DET_IDLE;
			timer->SignalDetTimeSlice->Stop();
			Signal_Det_Info.State = SIGNAL_DET_IDLE;
			Signal_Det_Info.LPMHandle->TaskSetStatus(SIGNAL_DET_TASK_ID, LPM_TASK_STA_STOP);
			Signal_DetTaskStopHandler(NULL); /* 信号检测任务停止处理 */
			break;
	}
	
	return;	
}

Signal_Det_Info_t* Signal_DetTaskInit(LPM_t* LPMHandle)
{
	SignalDetState = SIGNAL_DET_IDLE;
	Signal_Det_Info.State = SIGNAL_DET_IDLE;	
	Signal_Det_Info.ActiveTime = SIGNAL_DET_TIME_SLICE;
	
	Signal_Det_Info.LPMHandle = LPMHandle;
	
	Signal_Det_Info.TaskStart = Signal_DetTaskStart;
	Signal_Det_Info.TaskStop = Signal_DetTaskStop;
	Signal_Det_Info.StatusProc = Signal_Detect;
	
	Signal_Det_Info.LPMHandle->TaskRegister(SIGNAL_DET_TASK_ID);
	
	return &Signal_Det_Info;
}

/* 信号检测任务停止处理 */
__weak void Signal_DetTaskStopHandler(void* param)
{
	return;
}




