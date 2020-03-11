#include "sw_timer_rtc.h"


static SWT_t SWTHandle;


/**********************************系统空闲软件定时器**********************************/
static SWTimer_t SWTSysIdle;
APP_TIMER_DEF(sys_idle_id);
__weak void SWT_IdleCallback(void* param)
{
	return;
}

static void SWT_SysIdleStart(uint32_t time)
{
	app_timer_start(sys_idle_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_SysIdleStop(void)
{
	app_timer_stop(sys_idle_id);
}

static void SWT_SysIdleDelete(void)
{
	return;
}

SWTimer_t* SWT_SysIdleCreate(void)
{
	SWTSysIdle.ProcessID = 120;
	SWTSysIdle.Mode = APP_TIMER_MODE_SINGLE_SHOT/*APP_TIMER_MODE_REPEATED*/;
	SWTSysIdle.Callback = SWT_IdleCallback;
	SWTSysIdle.Start = SWT_SysIdleStart;
	SWTSysIdle.Stop = SWT_SysIdleStop;
	SWTSysIdle.Delete = SWT_SysIdleDelete;
	
	app_timer_create(&sys_idle_id,
									 SWTSysIdle.Mode,
									 SWTSysIdle.Callback);
	
	return &SWTSysIdle;
}


/**********************************系统低功耗时间软件定时器**********************************/
static SWTimer_t SWTSysLowPower;
APP_TIMER_DEF(sys_low_power_id);
__weak void SWT_SysLowPowerCallback(void* param)
{
	return;
}

static void SWT_SysLpStart(uint32_t time)
{
	SWT_SysIdleStop();
	app_timer_start(sys_low_power_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_SysLpStop(void)
{
	app_timer_stop(sys_low_power_id);
}

static void SWT_SysLpDelete(void)
{
	return;
}

SWTimer_t* SWT_SysLpCreate(void)
{
	SWTSysLowPower.ProcessID = 100;
	SWTSysLowPower.Mode = APP_TIMER_MODE_SINGLE_SHOT/*APP_TIMER_MODE_REPEATED*/;
	SWTSysLowPower.Callback = SWT_SysLowPowerCallback;
	SWTSysLowPower.Start = SWT_SysLpStart;
	SWTSysLowPower.Stop = SWT_SysLpStop;
	SWTSysLowPower.Delete = SWT_SysLpDelete;
	
	app_timer_create(&sys_low_power_id,
									 SWTSysLowPower.Mode,
									 SWTSysLowPower.Callback);
	
	return &SWTSysLowPower;
}


/**********************************蓝牙广播时间软件定时器**********************************/
static SWTimer_t SWTBleAdv;
APP_TIMER_DEF(ble_adv_id);
__weak void SWT_BleAdvCallback(void* param)
{
	return;
}

static void SWT_BleAdvStart(uint32_t time)
{
	SWT_SysIdleStop();
	app_timer_start(ble_adv_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_BleAdvStop(void)
{
	app_timer_stop(ble_adv_id);
}

static void SWT_BleAdvDelete(void)
{
	return;
}

SWTimer_t* SWT_BleAdvCreate(void)
{
	SWTBleAdv.ProcessID = 101;
	SWTBleAdv.Mode = APP_TIMER_MODE_SINGLE_SHOT/*APP_TIMER_MODE_REPEATED*/;
	SWTBleAdv.Callback = SWT_BleAdvCallback;
	SWTBleAdv.Start = SWT_BleAdvStart;
	SWTBleAdv.Stop = SWT_BleAdvStop;
	SWTBleAdv.Delete = SWT_BleAdvDelete;
	
	app_timer_create(&ble_adv_id,
									 SWTBleAdv.Mode,
									 SWTBleAdv.Callback);
	
	return &SWTBleAdv;
}


/**********************************蓝牙广播LED指示灯软件定时器**********************************/
static SWTimer_t SWTBleAdvLed;
APP_TIMER_DEF(ble_adv_led_id);
__weak void SWT_BleAdvLedCallback(void* param)
{
	return;
}

static void SWT_BleAdvLedStart(uint32_t time)
{
	SWT_SysIdleStop();
	app_timer_start(ble_adv_led_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_BleAdvLedStop(void)
{
	app_timer_stop(ble_adv_led_id);
}

static void SWT_BleAdvLedDelete(void)
{
	return;
}

SWTimer_t* SWT_SignalDetTimeoutCreate(void)
{
	SWTBleAdvLed.ProcessID = 106;
	SWTBleAdvLed.Mode = APP_TIMER_MODE_REPEATED/*APP_TIMER_MODE_SINGLE_SHOT*/;
	SWTBleAdvLed.Callback = SWT_BleAdvLedCallback;
	SWTBleAdvLed.Start = SWT_BleAdvLedStart;
	SWTBleAdvLed.Stop = SWT_BleAdvLedStop;
	SWTBleAdvLed.Delete = SWT_BleAdvLedDelete;
	
	app_timer_create(&ble_adv_led_id,
					 SWTBleAdvLed.Mode,
					 SWTBleAdvLed.Callback);

	return &SWTBleAdvLed;
}


/**********************************LORA活动时间片软件定时器**********************************/
static SWTimer_t SWTLoraTaskTimeSlice;
APP_TIMER_DEF(lora_task_time_slice_id);
__weak void SWT_LoraTaskTimeSliceCallback(void* param)
{
	return;
}

static void SWT_LoraTaskTimeSliceStart(uint32_t time)
{
	SWT_SysIdleStop();
	app_timer_start(lora_task_time_slice_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_LoraTaskTimeSliceStop(void)
{
	app_timer_stop(lora_task_time_slice_id);
}

static void SWT_LoraTaskTimeSliceDelete(void)
{
	return;
}

SWTimer_t* SWT_LoraTaskTimeSliceCreate(void)
{
	SWTLoraTaskTimeSlice.ProcessID = 102;
	SWTLoraTaskTimeSlice.Mode = APP_TIMER_MODE_SINGLE_SHOT/*APP_TIMER_MODE_REPEATED*/;
	SWTLoraTaskTimeSlice.Callback = SWT_LoraTaskTimeSliceCallback;
	SWTLoraTaskTimeSlice.Start = SWT_LoraTaskTimeSliceStart;
	SWTLoraTaskTimeSlice.Stop = SWT_LoraTaskTimeSliceStop;
	SWTLoraTaskTimeSlice.Delete = SWT_LoraTaskTimeSliceDelete;
	
	app_timer_create(&lora_task_time_slice_id,
									 SWTLoraTaskTimeSlice.Mode,
									 SWTLoraTaskTimeSlice.Callback);
	
	return &SWTLoraTaskTimeSlice;
}


/**********************************LORA数据发送超时软件定时器**********************************/
static SWTimer_t SWTLoraTxTimeout;
APP_TIMER_DEF(lora_tx_timeout_id);
__weak void SWT_LoraTxTimeoutCallback(void* param)
{
	return;
}

static void SWT_LoraTxTimeoutStart(uint32_t time)
{
	SWT_SysIdleStop();
	app_timer_start(lora_tx_timeout_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_LoraTxTimeoutStop(void)
{
	app_timer_stop(lora_tx_timeout_id);
}

static void SWT_LoraTxTimeoutDelete(void)
{
	return;
}

SWTimer_t* SWT_LoraTxTimeoutCreate(void)
{
	SWTLoraTxTimeout.ProcessID = 103;
	SWTLoraTxTimeout.Mode = APP_TIMER_MODE_SINGLE_SHOT/*APP_TIMER_MODE_REPEATED*/;
	SWTLoraTxTimeout.Callback = SWT_LoraTxTimeoutCallback;
	SWTLoraTxTimeout.Start = SWT_LoraTxTimeoutStart;
	SWTLoraTxTimeout.Stop = SWT_LoraTxTimeoutStop;
	SWTLoraTxTimeout.Delete = SWT_LoraTxTimeoutDelete;
	
	app_timer_create(&lora_tx_timeout_id,
									 SWTLoraTxTimeout.Mode,
									 SWTLoraTxTimeout.Callback);

	return &SWTLoraTxTimeout;
}


/**********************************LORA空闲时间软件定时器**********************************/
static SWTimer_t SWTLoraIdle;
APP_TIMER_DEF(lora_idle_id);
__weak void SWT_LoraIdleCallback(void* param)
{
	return;
}

static void SWT_LoraIdleStart(uint32_t time)
{
	SWT_SysIdleStop();
	app_timer_start(lora_idle_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_LoraIdleStop(void)
{
	app_timer_stop(lora_idle_id);
}

static void SWT_LoraIdleDelete(void)
{
	return;
}

SWTimer_t* SWT_LoraIdleCreate(void)
{
	SWTLoraIdle.ProcessID = 104;
	SWTLoraIdle.Mode = APP_TIMER_MODE_SINGLE_SHOT/*APP_TIMER_MODE_REPEATED*/;
	SWTLoraIdle.Callback = SWT_LoraIdleCallback;
	SWTLoraIdle.Start = SWT_LoraIdleStart;
	SWTLoraIdle.Stop = SWT_LoraIdleStop;
	SWTLoraIdle.Delete = SWT_LoraIdleDelete;
	
	app_timer_create(&lora_idle_id,
									 SWTLoraIdle.Mode,
									 SWTLoraIdle.Callback);

	return &SWTLoraIdle;
}


/**********************************信号检测时间片软件定时器**********************************/
static SWTimer_t SWTSignalDetTimeSlice;
APP_TIMER_DEF(signal_det_time_slice_id);
__weak void SWT_SignalDetTimeSliceCallback(void* param)
{
	return;
}

static void SWT_SignalDetTimeSliceStart(uint32_t time)
{
	SWT_SysIdleStop();
	app_timer_start(signal_det_time_slice_id, APP_TIMER_TICKS(time), NULL);
}

static void SWT_SignalDetTimeSliceStop(void)
{
	app_timer_stop(signal_det_time_slice_id);
}

static void SWT_SignalDetTimeSliceDelete(void)
{
	return;
}

SWTimer_t* SWT_SignalDetTimeSliceCreate(void)
{
	SWTSignalDetTimeSlice.ProcessID = 105;
	SWTSignalDetTimeSlice.Mode = APP_TIMER_MODE_SINGLE_SHOT/*APP_TIMER_MODE_REPEATED*/;
	SWTSignalDetTimeSlice.Callback = SWT_SignalDetTimeSliceCallback;
	SWTSignalDetTimeSlice.Start = SWT_SignalDetTimeSliceStart;
	SWTSignalDetTimeSlice.Stop = SWT_SignalDetTimeSliceStop;
	SWTSignalDetTimeSlice.Delete = SWT_SignalDetTimeSliceDelete;
	
	app_timer_create(&signal_det_time_slice_id,
									 SWTSignalDetTimeSlice.Mode,
									 SWTSignalDetTimeSlice.Callback);

	return &SWTSignalDetTimeSlice;
}


void SWT_Init(void)
{
	SWTHandle.SysIdle = &SWTSysIdle;
	SWTHandle.SysLowPower = &SWTSysLowPower;
	SWTHandle.BleAdv = &SWTBleAdv;
	SWTHandle.BleAdvLed = &SWTBleAdvLed;
	SWTHandle.LoraTaskTimeSlice = &SWTLoraTaskTimeSlice;
	SWTHandle.LoraTxTimeout = &SWTLoraTxTimeout;
	SWTHandle.LoraIdle = &SWTLoraIdle;
	SWTHandle.SignalDetTimeSlice = &SWTSignalDetTimeSlice;
	
	SWT_SysIdleCreate();
	SWT_SysLpCreate();
	SWT_BleAdvCreate();
	SWT_LoraTaskTimeSliceCreate();
	SWT_LoraTxTimeoutCreate();
	SWT_LoraIdleCreate();
	SWT_SignalDetTimeSliceCreate();
	SWT_SignalDetTimeoutCreate();
}

SWT_t* SWT_GetHandle(void)
{
	return &SWTHandle;
}










