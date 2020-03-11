#include "low_power_manage.h"
#include "nrf_pwr_mgmt.h"
#include "light.h"


static uint8_t LPMTaskNumber = 0;
static LPM_t LPMHandle;
static LPM_Task_t LPMTaskTb[LPM_TASK_MAX_NUMBER];


static void LPM_TaskTbInit(void)
{
	for(int i=0; i<LPM_TASK_MAX_NUMBER; i++)
	{
		LPMTaskTb[i].TaskStatus = LPM_TASK_NULL;
	}
	
	return;
}

static ErrorStatus LPM_TaskRegister(uint8_t TaskId)
{
	ErrorStatus status = ERROR;
	
	if(LPMTaskNumber < LPM_TASK_MAX_NUMBER)
	{
		if(LPMTaskTb[LPMTaskNumber].TaskStatus == LPM_TASK_NULL)
		{
			LPMTaskTb[LPMTaskNumber].TaskId = TaskId;
			LPMTaskTb[LPMTaskNumber].TaskStatus = LPM_TASK_REGISTER;
			LPMTaskNumber += 1;
			status = SUCCESS;
		}
	}
	
	return status;
}

static ErrorStatus LPM_TaskCancelRegister(uint8_t TaskId)
{
	uint8_t i;
	ErrorStatus status = ERROR;
	
	for(i=0; i<LPMTaskNumber; i++)
	{
		if(LPMTaskTb[i].TaskId == TaskId)
		{
			LPMTaskTb[i].TaskStatus = LPM_TASK_NULL;
			status = SUCCESS;
			break;
		}
	}
	
	if(status == SUCCESS)
	{
		for(i+=1; i<LPMTaskNumber; i++)
		{
			if(LPMTaskTb[i].TaskStatus == LPM_TASK_NULL)
			{
				break;
			}

			LPMTaskTb[i-1].TaskId = LPMTaskTb[i].TaskId;
			LPMTaskTb[i-1].TaskStatus = LPMTaskTb[i].TaskStatus;
		}
		
		LPMTaskNumber -= 1;
	}

	return status;
}

static ErrorStatus LPM_TaskSetStatus(uint8_t TaskId, uint8_t state)
{
	if(!IS_LPM_TASK_STATE(state))
	{
		return ERROR;
	}
	
	ErrorStatus status = ERROR;
	for(int i=0; i<LPMTaskNumber; i++)
	{
		if(LPMTaskTb[i].TaskId == TaskId)
		{
			LPMTaskTb[i].TaskStatus = state;
			status = SUCCESS;
			break;
		}
	}
	
	return status;
}

static uint8_t LPM_TaskQueryStatus(uint8_t TaskId)
{
	uint8_t status = LPM_TASK_STA_INVALID;
	
	for(int i=0; i<LPMTaskNumber; i++)
	{
		if(LPMTaskTb[i].TaskId == TaskId)
		{
			status = LPMTaskTb[i].TaskStatus;
			break;
		}
	}
	
	return status;
}

static FlagStatus LPM_IsTaskRun(void)
{
	FlagStatus status = RESET;
	
	for(int i=0; i<LPMTaskNumber; i++)
	{
		if(LPMTaskTb[i].TaskStatus == LPM_TASK_STA_RUN)
		{
			status = SET;
			break;
		}
	}
	
	return status;
}

void SWT_IdleCallback(void)
{
	return;
}

static void LPM_Process(lpm_handler_t enter, lpm_handler_t exit)
{
	/* 查询是否有任务正在运行 */
	if(LPM_IsTaskRun() == SET)
	{
		return;
	}
	
	/* 如果没有任何任务需要运行则启动空闲任务以定期唤醒系统 */
	if(RTC_STATUS_GET() == RTC_STOP)
	{
		SWT_t* timer = SWT_GetHandle();
		timer->SysIdle->Start(5000);
	}
	
	if(enter)
	{
		enter();
	}
	
	nrf_pwr_mgmt_run();
	
	if(exit)
	{
		exit();
	}
	
	return;
}

static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

LPM_t* LPM_Init(void)
{
	power_management_init();
	
	LPM_TaskTbInit();
	LPMHandle.TaskRegister = LPM_TaskRegister;
	LPMHandle.TaskCancelRegister = LPM_TaskCancelRegister;
	LPMHandle.TaskSetStatus = LPM_TaskSetStatus;
	LPMHandle.TaskQueryStatus = LPM_TaskQueryStatus;
	LPMHandle.IsTaskRun = LPM_IsTaskRun;
	LPMHandle.LowPowerManage = LPM_Process;
	
	return &LPMHandle;
}




