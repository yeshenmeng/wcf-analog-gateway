#include "sys_proc.h"
#include "low_power_manage.h"
#include "sys_param.h"
#include "light.h"


#define CHECK_TASK_EVT(evt)			(SysTaskEvt & evt)
#define SET_TASK_EVT(evt)			(SysTaskEvt |= evt)
#define CLEAR_TASK_EVT(evt)			(SysTaskEvt &= ~evt)

//#define SIGNAL_DET_SW
//#define SIGNAL_DET_HW

typedef struct {
	Sys_Task_t Task;
}Sys_Info_t;

static uint32_t SysTaskEvt;
static Sys_Info_t Sys_Info;

void Sys_TaskInit(Sys_Task_t* task)
{
	Sys_Info.Task.BleTask = task->BleTask;
	Sys_Info.Task.LoraTask = task->LoraTask;
	Sys_Info.Task.SlpTask = task->SlpTask;
	Sys_Info.Task.InclinometerTask = task->InclinometerTask;
	Sys_Info.Task.SignalDetTask = task->SignalDetTask;
}

void Sys_Startup(void)
{
#ifdef SIGNAL_DET_SW
	SET_TASK_EVT(SYS_TASK_EVT_BLE);
#endif
	
	sys_param_t* param = Sys_ParamGetHandle();
	if(*(uint64_t*)param->dev_gateway_addr!=0 &&
	   *(uint64_t*)param->dev_gateway_addr!=(~0))
	{
		SET_TASK_EVT(SYS_TASK_EVT_LORA);
	}
	else
	{
#ifdef SIGNAL_DET_HW
		SET_TASK_EVT(SYS_TASK_EVT_BLE);
#endif
	}
	
//	*(uint64_t*)Sys_Info.Param.LoraGatewayLongAddr = 0X01;
//	SET_TASK_EVT(SYS_TASK_EVT_LORA);
//	SET_TASK_EVT(SYS_TASK_EVT_SIGNAL_DET);
//	SET_TASK_EVT(SYS_TASK_EVT_INCLINOMETER);
//	SET_TASK_EVT(SYS_TASK_EVT_BLE);
}

void Sys_TaskScheduler(void)
{
	if(CHECK_TASK_EVT(SYS_TASK_EVT_INIT))
	{
		CLEAR_TASK_EVT(SYS_TASK_EVT_INIT);
		//初始化LORA参数等
	}
	
	if(CHECK_TASK_EVT(SYS_TASK_EVT_BLE))
	{
		CLEAR_TASK_EVT(SYS_TASK_EVT_BLE);
		Sys_Info.Task.BleTask->TaskStart(); /* 启动蓝牙任务 */
	}
	
	if(CHECK_TASK_EVT(SYS_TASK_EVT_INCLINOMETER))
	{
		CLEAR_TASK_EVT(SYS_TASK_EVT_INCLINOMETER);
		Sys_Info.Task.InclinometerTask->TaskStart(); /* 启动倾角传感器采样任务 */
	}
	
	if(CHECK_TASK_EVT(SYS_TASK_EVT_LORA))
	{
		CLEAR_TASK_EVT(SYS_TASK_EVT_LORA);
		
		sys_param_t* param = Sys_ParamGetHandle();
		if(*(uint64_t*)param->dev_gateway_addr!=0 &&
		   *(uint64_t*)param->dev_gateway_addr!=(~0))
		{
			Sys_Info.Task.LoraTask->TaskStart(); /* 启动LORA任务 */
		}
	}
	
	if(CHECK_TASK_EVT(SYS_TASK_EVT_SYS_LP))
	{
		CLEAR_TASK_EVT(SYS_TASK_EVT_SYS_LP);
		Sys_Info.Task.SlpTask->TaskStart(); /* 启动系统低功耗任务 */
	}
	
	if(CHECK_TASK_EVT(SYS_TASK_EVT_SIGNAL_DET))
	{
		CLEAR_TASK_EVT(SYS_TASK_EVT_SIGNAL_DET);
		Sys_Info.Task.SignalDetTask->TaskStart(); /* 启动信号检测任务 */
	}
}
uint32_t ble_stop_cnt = 0;
uint32_t in_stop_cnt = 0;
uint32_t lora_stop_cnt = 0;
uint32_t signal_stop_cnt = 0;
uint32_t slp_stop_cnt = 0;
uint32_t lora_disconn_disconn_cnt = 0;
uint32_t lora_conn_disconn_cnt = 0;
uint32_t lora_disconn_conn_cnt = 0;
uint32_t lora_conn_conn_cnt = 0;
void BLE_TaskStopHandler(void* param)
{
	Ble_State bleState = *(Ble_State*)param;
	ble_stop_cnt++;
//	if(bleState == BLE_STA_ADV_TIMEOUT)
//	{
//		/* 设置LORA默认网关地址 */
//		sys_param_t* param = Sys_ParamGetHandle();
//		if(*(uint64_t*)param->dev_gateway_addr==0 ||
//		   *(uint64_t*)param->dev_gateway_addr==(~0))
//		{
//			uint8_t gateway_addr[] = SYS_PARAM_DEV_GATEWAY_ADDR;
//			sys_param_set((uint8_t*)&param->dev_gateway_addr, gateway_addr, sizeof(gateway_addr));
//			SET_TASK_EVT(SYS_TASK_EVT_LORA); //启动LORA传输任务
//		}
//	}
	
	/* 启动信号检测任务 */
	SET_TASK_EVT(SYS_TASK_EVT_BLE);
}

void Inclinometer_TaskStopHandler(void)
{
	/* 启动LORA传输任务 */
	SET_TASK_EVT(SYS_TASK_EVT_LORA);
	in_stop_cnt++;
}

void LORA_TaskStopHandler(void* param)
{
	SET_TASK_EVT(SYS_TASK_EVT_LORA);
	
//	uint8_t loraState = *(uint8_t*)param;
//	lora_stop_cnt++;
//	/* LORA从未连接态到未连接态 */
//	if(loraState == LORA_OUT_STATE_OFFLINE)
//	{
//		/* 清空网关并保存 */
//		sys_param_t* param = Sys_ParamGetHandle();
//		uint8_t gateway_addr[8] = {0};
//		sys_param_set((uint8_t*)&param->dev_gateway_addr, gateway_addr, sizeof(gateway_addr));
//		lora_disconn_disconn_cnt++;
//		/* 启动蓝牙任务重新配置网关 */
//		SET_TASK_EVT(SYS_TASK_EVT_BLE);
//	}
//	/* LORA从连接态到未连接态 */
//	else if(loraState == LORA_OUT_STATE_DISCON)
//	{
//		/* 重新连接网关 */
//		SET_TASK_EVT(SYS_TASK_EVT_LORA);
//		lora_conn_disconn_cnt++;
//	}
//	/* LORA从未连接态到连接态 */
//	else if(loraState == LORA_OUT_STATE_LINK)
//	{
//		/* 连接网关成功设置倾角任务运行 */
//		SET_TASK_EVT(SYS_TASK_EVT_INCLINOMETER);
//		lora_disconn_conn_cnt++;
//	}
//	/* LORA从连接态到连接态 */
//	else if(loraState == LORA_OUT_STATE_CONNECT)
//	{
//		/* LORA数据推送成功设置系统低功耗任务运行 */
//		SET_TASK_EVT(SYS_TASK_EVT_SYS_LP);
//		lora_conn_conn_cnt++;
//	}
}

/* 硬件信号检测事件 */
void Signal_DetEvtHandler(void* param)
{
	/* 检测到蓝牙信号启动蓝牙任务 */
	SET_TASK_EVT(SYS_TASK_EVT_BLE);
	signal_stop_cnt++;
}

/* 软件信号检测事件 */
void Signal_DetTaskStopHandler(void* param)
{
	/* 启动蓝牙任务 */
	SET_TASK_EVT(SYS_TASK_EVT_BLE);
	signal_stop_cnt++;
}

void SLP_TaskStopHandler(void* param)
{
	/* 启动倾角任务 */
	SET_TASK_EVT(SYS_TASK_EVT_INCLINOMETER);
	slp_stop_cnt++;
}
















