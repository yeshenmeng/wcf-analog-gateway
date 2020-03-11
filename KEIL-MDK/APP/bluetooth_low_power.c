#include "bluetooth_low_power.h"
#include "sw_timer_rtc.h"
#include "sys_param.h"
#include "light.h"


static uint8_t BleRssiFlag = 0;
static Ble_State BleState;
static Ble_Info_t BleInfo;

__weak void BLE_TaskStopHandler(void* param);
__weak void BLE_StaErrHandler(void* param);


static void Ble_TaskStart(void)
{
	if(BleInfo.State == BLE_STA_IDLE)
	{
		BleInfo.LPMHandle->TaskSetStatus(BLE_TASK_ID, LPM_TASK_STA_RUN);
		BleState = BLE_STA_ADV;
	}
	return;
}

static void Ble_TaskStop(void)
{
	if(BleInfo.State != BLE_STA_IDLE)
	{
		if(BleInfo.State == BLE_STA_ADV)
		{
			BleState = BLE_STA_ADV_STOP;
		}
		else if(BleInfo.State == BLE_STA_CONN)
		{
			BleState = BLE_STA_CONN_ABORT;
		}
	}
	return;
}

static ErrorStatus Ble_StartAdv(void)
{
	ErrorStatus status = SUCCESS;
	ble_adv_start();
	return status;
}

static ErrorStatus Ble_StopAdv(void)
{
	ErrorStatus status = SUCCESS;
	ble_adv_stop();
	return status;
}

static ErrorStatus Ble_AbortConn(void)
{
	ErrorStatus status = SUCCESS;
	ble_disconnect();
	return status;
}

static void Ble_GetAddress(uint8_t* addr)
{
	ble_get_address(addr);
	return;
}

static ErrorStatus Ble_SetTxPower(uint8_t level)
{
	ErrorStatus status = SUCCESS;
	ble_tx_power_set(level);
	return status;
}

static void Ble_IdleLowPowerManage(void)
{
	if(BleInfo.Param.IsIdleEnterLp == 0)
	{
		return;
	}
	
	if(BleInfo.State != BLE_STA_IDLE)
	{
		BleInfo.LPMHandle->TaskSetStatus(BLE_TASK_ID, LPM_TASK_STA_LP); /* 设置任务低功耗状态 */
	}
}

static ErrorStatus Ble_ReadRSSI(int8_t* rssi)
{
	ErrorStatus status = SUCCESS;
	uint8_t* ch_index;
	ble_rssi_get(rssi, ch_index);
	return status;
}

void SWT_BleAdvCallback(void)
{
	BleState = BLE_STA_ADV_TIMEOUT;
	return;
}

void SWT_BleAdvLedCallback(void)
{
	if(BleInfo.State == BLE_STA_CONN)
	{
		BleRssiFlag = 1;
	}
	
	LIGHT_2_TOGGLE();
	return;
}

void ble_conn_handler(void)
{
	BleState = BLE_STA_CONN;
	return;
}

void ble_disconn_handler(void)
{
	BleState = BLE_STA_DISCON;
	return;
}

static void Ble_StatusProc(void)
{
	Ble_State stateTmp;
	ErrorStatus status = SUCCESS;
	SWT_t* timer = SWT_GetHandle();
	
	switch((uint8_t)BleState)
	{
		/* 蓝牙开启广播 */
		case BLE_STA_ADV:
		{
			status = Ble_StartAdv(); /* 开始蓝牙广播 */
			sys_param_t* param = Sys_ParamGetHandle();
			ble_tx_power_set(param->ble_tx_power);
			if(param->ble_adv_time != 0)
			{
				timer->BleAdv->Start(param->ble_adv_time*1000); /* 启动蓝牙广播定时器 */
			}
			timer->BleAdvLed->Start(param->ble_adv_interval); /* 启动BLE广播LED指示灯定时器 */
			Ble_GetAddress(BleInfo.Param.Address); /* 获取蓝牙地址 */
			BleInfo.State = BleState;
			BleState = BLE_STA_IDLE;
			Ble_IdleLowPowerManage();
			break;
		}
		
		/* 蓝牙连接事件 */
		case BLE_STA_CONN:
		{
			timer->BleAdv->Stop(); /* 停止BLE广播定时器 */
			timer->BleAdvLed->Stop();
			sys_param_t* param = Sys_ParamGetHandle();
			timer->BleAdvLed->Start((param->ble_min_conn_interval+param->ble_max_conn_interval)/2);
			LIGHT_2_ON();
			BleInfo.State = BleState;
			BleState = BLE_STA_IDLE;
			break;
		}

		/* 蓝牙被动断开连接事件 */
		case BLE_STA_DISCON:
			timer->BleAdvLed->Stop();
			LIGHT_2_OFF();
			BleInfo.State = BLE_STA_IDLE; /* 更新蓝牙状态为未连接 */
			BleState = BLE_STA_ADV;
			BleInfo.LPMHandle->TaskSetStatus(BLE_TASK_ID, LPM_TASK_STA_RUN); 
			break;

		/* 蓝牙广播超时事件 */
		case BLE_STA_ADV_TIMEOUT:
		/* 停止蓝牙广播 */
		case BLE_STA_ADV_STOP:
			stateTmp = BleState;
			status = Ble_StopAdv(); /* 停止蓝牙广播 */
			timer->BleAdv->Stop(); /* 停止BLE广播定时器 */
			timer->BleAdvLed->Stop(); /* 停止BLE广播LED指示灯定时器 */
			LIGHT_2_OFF();
			BleInfo.State = BLE_STA_IDLE;
			BleState = BLE_STA_IDLE;
			BleInfo.LPMHandle->TaskSetStatus(BLE_TASK_ID, LPM_TASK_STA_STOP);
			BLE_TaskStopHandler((void *)&stateTmp);
			break;
		
		/* 主动断开连接 */
		case BLE_STA_CONN_ABORT:
			status = BleInfo.AbortConn(); /* 断开蓝牙连接 */
			BleInfo.State = BleState;
			BleState = BLE_STA_IDLE;
			break;
	}
	
	if(status != SUCCESS)
	{
		BLE_StaErrHandler((void *)&BleState); /* 状态错误处理 */
	}
	
	if(BleInfo.State == BLE_STA_CONN && BleRssiFlag == 1)
	{
		BleRssiFlag = 0;
		Ble_ReadRSSI(&BleInfo.Param.RSSI); /* 读取蓝牙信号强度 */
	}
}

Ble_Info_t* Ble_Init(LPM_t* LPMHandle)
{
	BleInfo.Param.IsIdleEnterLp = DISABLE;
	
	BleState = BLE_STA_IDLE;
	BleInfo.State = BleState;
	
	BleInfo.LPMHandle = LPMHandle;
	
	BleInfo.StatusProc = Ble_StatusProc;
	BleInfo.TaskStart = Ble_TaskStart;
	BleInfo.TaskStop = Ble_TaskStop;
	BleInfo.AbortConn = Ble_AbortConn;
	BleInfo.ReadRSSI = Ble_ReadRSSI;
	BleInfo.SetTxPower = Ble_SetTxPower;
	
	BleInfo.LPMHandle->TaskRegister(BLE_TASK_ID);
	
	return &BleInfo;
}

/* 蓝牙活动停止处理 */
__weak void BLE_TaskStopHandler(void* param)
{
	 return;
}

/* 蓝牙状态错误处理 */
__weak void BLE_StaErrHandler(void* param)
{
	return;
}







