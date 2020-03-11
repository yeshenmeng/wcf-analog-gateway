/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief Blinky Sample Application main file.
 *
 * This file contains the source code for a sample server application using the LED Button service.
 */

#include <stdint.h>
#include <string.h>
#include "main.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "nrf_sdh.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


/* USER CODE BEGIN Includes */
#include "ble_init.h"
#include "low_power_manage.h"
#include "sw_timer_rtc.h"
#include "bluetooth_low_power.h"
#include "rng_lpm.h"
#include "lora_transmission.h"
#include "sys_proc.h"
#include "inclinometer.h"
#include "iotobject.h"
#include "wireless_comm_services.h"
#include "signal_detect.h"
#include "sw_signal_detect.h"
#include "flash.h"
#include "iot_operate.h"
#include "sys_param.h"
#include "calendar.h"
#include "uart_svc.h"
#include "light.h"
#include "ble_char_handler.h"
/* USER CODE END Includes */


#define DEAD_BEEF	0xDEADBEEF	/**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

static void LFCLK_Config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

/* USER CODE BEGIN 0 */
wireless_comm_services_t* WirelessCommSvc;
LPM_t* LPMHandle;
Ble_Info_t *BLEHandle;
Lora_Info_t* LORAHandle;
SLP_Info_t* SLPHandle;
Inclinometer_Info_t* InclinometerHandle;
IoT_dev_t* IoTDevHandle;
Signal_Det_Info_t*  SignalDetHandle;

/**
 * @brief  初始化物联网协议属性长度
 * @param  sensor: 指向iot_object_t结构的句柄
 * @retval None
 */
void IoT_SetProp(iot_object_t *sensor) {
	//init addr, long addr, short addr are fixed. No need to set
	sensor->setPropCount(9);
	sensor->setPropLen(SAMPLE_INTERVAL_ID, 4);
	sensor->setPropLen(TIME_STAMP_ID, 4);
	sensor->setPropLen(BATTERY_LEVEL_ID, 1);
	sensor->setPropLen(TEMPERATURE_ID, 4);
	sensor->setPropLen(DATA_X_ANGLE_ID, 4);
	sensor->setPropLen(DATA_Y_ANGLE_ID, 4);
}

void LPM_EnterHandler(void)
{
	if(nrf_sdh_is_enabled() == true)
	{
		sd_power_dcdc_mode_set(0);
	}
	else
	{
		NRF_POWER->DCDCEN = 0; //低功耗关闭DCDC
	}

	if(BLEHandle->State == BLE_STA_IDLE)
	{
		LIGHT_OFF();
		SIGNAL_DET_PWOER_ENBALE();
	}
}

void LPM_ExitHandler(void)
{
	if(nrf_sdh_is_enabled() == true)
	{
		sd_power_dcdc_mode_set(1);
	}
	else
	{
		NRF_POWER->DCDCEN = 1; //退出低功耗打开DCDC
	}
	
	SIGNAL_DET_PWOER_DISABLE();
}

/* USER CODE END 0 */
uint32_t LedCnt = 0;
/**@brief Function for application main entry.
 */
int main(void)
{
	/**********************************硬件设备初始化**********************************/
	NRF_POWER->DCDCEN = 1; //打开DCDC
//	NRF_CLOCK->TASKS_HFCLKSTART = 1; //启动外部时钟源
//	while(!NRF_CLOCK->EVENTS_HFCLKSTARTED); //等待外部时钟源启动完成
	LFCLK_Config(); //RTC时钟源设置
	Light_Init(); //设备指示灯初始化
	timers_init(); //定时器初始化基于RTC1
	fs_flash_init(); //flash初始化
	
	/**********************************功能模块初始化**********************************/
	iot_object_t *mySensor = createSensorHandler(); //传感器对象初始化
	IoT_SetProp(mySensor);
	mySensor->init();
	WirelessCommSvc = createWirelessCommServiceHandler(); //无线通信服务初始化
	WirelessCommSvc->setSensorHandler(mySensor);
	Sys_ParamInit(); //系统参数初始化
//	Signal_DetInit(); //信号检测功能初始化
	SWT_Init(); //软件定时器初始化
	Calendar_Init(); //日历功能初始化初始化
	RNG_LPM_Init(); //低功耗随机数发生器初始化

	/**********************************BLE初始化**********************************/
	BLE_Init();

	/**********************************应用初始化**********************************/
	LPMHandle = LPM_Init(); //低功耗管理初始化
	BLEHandle = Ble_Init(LPMHandle); //BLE任务初始化	
	SignalDetHandle = Signal_DetTaskInit(LPMHandle); //软件信号检测任务初始化
	LORAHandle = LORA_TaskInit(LPMHandle); //LORA任务初始化
	SLPHandle = SLP_TaskInit(LPMHandle); //系统低功耗任务初始化
//	InclinometerHandle = Inclinometer_TaskInit(LPMHandle); //测斜仪任务初始化
	IoTDevHandle = IoT_Init(mySensor, InclinometerHandle); //物联网协议属性初始化
	
	/**********************************系统任务设置**********************************/
	Sys_Task_t task;
	task.BleTask = BLEHandle;
	task.InclinometerTask = InclinometerHandle;
	task.LoraTask = LORAHandle;
	task.SignalDetTask = SignalDetHandle;
	task.SlpTask = SLPHandle;
	Sys_TaskInit(&task); //系统任务初始化
	Sys_Startup(); //系统启动
	
	nrf_delay_ms(300);
	LIGHT_OFF();
	nrf_delay_ms(300);
	uart_init();
	
//	while(1) 
//	{
//		nrf_delay_ms(1000);
//		printf("ok!\n");
//	}

	while(1)
	{
		Sys_TaskScheduler();
		
		LPMHandle->LowPowerManage(LPM_EnterHandler, LPM_ExitHandler);
		
		IoTDevHandle->operate();
		
		BLEHandle->StatusProc();
		
		LORAHandle->StatusProc();
		
		SLPHandle->StatusProc();
		
//		InclinometerHandle->TaskOperate();
		
		SignalDetHandle->StatusProc();
		
//		mySensor->saveProp2Flash();
		
		Sys_SaveParamToFlash();
		
		ble_char_change_handler();

		LedCnt++;
		
		uart_run();
	}
}


/**
 * @}
 */
