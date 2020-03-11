#ifndef __SYS_PROC_H__
#define __SYS_PROC_H__
#include "main.h"
#include "bluetooth_low_power.h"
#include "lora_transmission.h"
#include "system_low_power.h"
#include "inclinometer.h"
#include "sw_signal_detect.h"


#define SYS_TASK_EVT_NULL						0X00u
#define SYS_TASK_EVT_INIT						0X01u
#define	SYS_TASK_EVT_BLE						0X02u
#define	SYS_TASK_EVT_INCLINOMETER				0X04u
#define	SYS_TASK_EVT_LORA						0X08u
#define	SYS_TASK_EVT_SYS_LP						0X10u
#define	SYS_TASK_EVT_SIGNAL_DET					0X20u

typedef struct {
	Ble_Info_t* BleTask;
	Lora_Info_t* LoraTask;
	SLP_Info_t* SlpTask;
	Inclinometer_Info_t* InclinometerTask;
	Signal_Det_Info_t* SignalDetTask;
}Sys_Task_t;

void Sys_TaskInit(Sys_Task_t* task);

void Sys_TaskScheduler(void);
void Sys_Startup(void);

#endif





