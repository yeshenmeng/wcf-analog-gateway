#ifndef __BLUETOOTH_LOW_POWER_H__
#define __BLUETOOTH_LOW_POWER_H__
#include "main.h"
#include "low_power_manage.h"
#include "ble_init.h"


#define BLE_TASK_ID						2
#define	BLE_MTU_SIZE					255 /* MTU大小 */


typedef enum {
	BLE_STA_IDLE,
	BLE_STA_ADV,
	BLE_STA_CONN,
	BLE_STA_DISCON,
	BLE_STA_ADV_TIMEOUT,
	BLE_STA_ADV_STOP,
	BLE_STA_CONN_ABORT,
	BLE_STA_KEEP_ALIVE,
	BLE_STA_ACT_IDLE,
}Ble_State;

/* 广播数据结构 */
typedef struct {
	uint8_t length;
	uint8_t type;
	uint8_t value[];
}AD_Struct_t;

typedef struct {
//	/* 配置参数 */
//	uint16_t MTU;
//	uint8_t SetAddress[6];
	
	/* 被动参数 */
	int8_t RSSI;
	uint8_t Address[6];
	
	/* 控制参数 */
	FunctionalState IsIdleEnterLp; /* DISABLE：蓝牙空闲状态不动作，ENABLE：蓝牙空闲状态进入低功耗模式 */
}Ble_Param_t;

typedef struct {
	Ble_State State;
	Ble_Param_t Param;
	
	LPM_t* LPMHandle;
	
	void (*TaskStart)(void);
	void (*TaskStop)(void);
	void (*StatusProc)(void);
	ErrorStatus (*AbortConn)(void);
	ErrorStatus (*SetTxPower)(uint8_t PALevel);
	ErrorStatus (*ReadRSSI)(int8_t* rssi);
}Ble_Info_t;

void Ble_ConfigInit(void);
Ble_Info_t* Ble_Init(LPM_t* LPMHandle);

#endif

