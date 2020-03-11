#include "ble_char_handler.h"
#include "sys_param.h"
#include "ble_init.h"
#include "sw_timer_rtc.h"
#include "string.h"


#define BLE_TX_POWER_CHANGE				0x01
#define BLE_ADV_INTERVAL_CHANGE			0x02
#define BLE_ADV_TIME_CHANGE				0x04
#define BLE_MIN_CONN_INTERVAL_CHANGE	0x08
#define BLE_MAX_CONN_INTERVAL_CHANGE	0x10
#define BLE_SLAVE_LATENCY_CHANGE		0x20
#define BLE_CONN_TIMEOUT_CHANGE			0x40

#define LORA_FREQ_CHANGE				0x01
#define LORA_POWER_CHANGE				0x02
#define LORA_BW_CHANGE					0x04
#define LORA_SF_CHANGE					0x08
#define LORA_CODE_RATE_CHANGE			0x10
#define LORA_PREAMBLE_CHANGE			0x20
#define LORA_HEADER_CHANGE				0x40
#define LORA_CRC_CHANGE					0x80

#define DEV_GATEWAY_ADDR_CHANGE			0x01
#define DEV_LONG_ADDR_CHANGE			0x02
#define DEV_SHORT_ADDR_CHANGE			0x04

static uint32_t ble_param_change = 0;
static uint32_t lora_param_change = 0;
static uint32_t dev_param_change = 0;

void ble_param_tx_power_write_handler(uint8_t* p_data, uint16_t len){
	ble_param_change |= BLE_TX_POWER_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->ble_tx_power, p_data, len);
}

void ble_param_adv_interval_write_handler(uint8_t* p_data, uint16_t len){
	ble_param_change |= BLE_ADV_INTERVAL_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->ble_adv_interval, p_data, len);
}

void ble_param_adv_time_write_handler(uint8_t* p_data, uint16_t len){
	ble_param_change |= BLE_ADV_TIME_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->ble_adv_time, p_data, len);
}

void ble_param_min_conn_interval_write_handler(uint8_t* p_data, uint16_t len){
	ble_param_change |= BLE_MIN_CONN_INTERVAL_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->ble_min_conn_interval, p_data, len);
}

void ble_param_max_conn_interval_write_handler(uint8_t* p_data, uint16_t len){
	ble_param_change |= BLE_MAX_CONN_INTERVAL_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->ble_max_conn_interval, p_data, len);
}

void ble_param_slave_latency_write_handler(uint8_t* p_data, uint16_t len){
	ble_param_change |= BLE_SLAVE_LATENCY_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->ble_slave_latency, p_data, len);
}

void ble_param_conn_timeout_write_handler(uint8_t* p_data, uint16_t len){
	ble_param_change |= BLE_CONN_TIMEOUT_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->ble_conn_timeout, p_data, len);
}
	
void ble_lora_freq_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_FREQ_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_freq, p_data, len);
}

void ble_lora_power_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_POWER_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_power, p_data, len);
}

void ble_lora_bw_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_BW_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_bw, p_data, len);
}

void ble_lora_sf_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_SF_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_sf, p_data, len);
}

void ble_lora_code_rate_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_CODE_RATE_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_code_rate, p_data, len);
}

void ble_lora_preamble_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_PREAMBLE_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_preamble, p_data, len);
}

void ble_lora_header_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_HEADER_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_header, p_data, len);
}

void ble_lora_crc_write_handler(uint8_t* p_data, uint16_t len){
	lora_param_change |= LORA_CRC_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->lora_crc, p_data, len);
}

void ble_dev_gateway_write_handler(uint8_t* p_data, uint16_t len){
	dev_param_change |= DEV_GATEWAY_ADDR_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->dev_gateway_addr, p_data, len);
}

void ble_dev_long_addr_write_handler(uint8_t* p_data, uint16_t len){
	dev_param_change |= DEV_LONG_ADDR_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->dev_long_addr, p_data, len);
}

void ble_dev_short_addr_write_handler(uint8_t* p_data, uint16_t len){
	dev_param_change |= DEV_SHORT_ADDR_CHANGE;
	sys_param_t* param = Sys_ParamGetHandle();
	sys_param_set((uint8_t*)&param->dev_short_addr, p_data, len);
}

uint8_t ble_conn_param_update_flag = 0;

void ble_conn_param_modify(void)
{
	if(ble_conn_param_update_flag == 1)
	{
		ble_conn_param_update_flag = 0;
		sys_param_t* param = Sys_ParamGetHandle();
		ble_conn_params_change(param->ble_min_conn_interval,
							   param->ble_max_conn_interval,
							   param->ble_slave_latency,
							   param->ble_conn_timeout);
	}
}

static void ble_cfg_char_change_handler(void)
{
	if(ble_param_change == 0)
	{
		return;
	}
	
	if(ble_param_change & BLE_TX_POWER_CHANGE)
	{
		ble_param_change &= ~BLE_TX_POWER_CHANGE;
	}
	
	if(ble_param_change & BLE_ADV_INTERVAL_CHANGE)
	{
		ble_param_change &= ~BLE_ADV_INTERVAL_CHANGE;
	}

	if(ble_param_change & BLE_ADV_TIME_CHANGE)
	{
		ble_param_change &= ~BLE_ADV_TIME_CHANGE;
	}

	if(ble_param_change & BLE_MIN_CONN_INTERVAL_CHANGE ||
	   ble_param_change & BLE_MAX_CONN_INTERVAL_CHANGE ||
	   ble_param_change & BLE_SLAVE_LATENCY_CHANGE)
	{
		ble_param_change &= ~BLE_MIN_CONN_INTERVAL_CHANGE;
		ble_param_change &= ~BLE_MAX_CONN_INTERVAL_CHANGE;
		ble_param_change &= ~BLE_SLAVE_LATENCY_CHANGE;
		
//		ble_conn_param_update_flag = 1;
		
//		sys_param_t* param = Sys_ParamGetHandle();
//		ble_conn_params_change(param->ble_min_conn_interval,
//							   param->ble_max_conn_interval,
//							   param->ble_slave_latency,
//							   param->ble_conn_timeout);
//		
//		SWT_t* timer = SWT_GetHandle();
//		timer->BleAdvLed->Stop();
//		timer->BleAdvLed->Start((param->ble_min_conn_interval+param->ble_max_conn_interval)/2);
	}
	
	if(ble_param_change & BLE_CONN_TIMEOUT_CHANGE)
	{
		ble_param_change &= ~BLE_CONN_TIMEOUT_CHANGE;
		sys_param_t* param = Sys_ParamGetHandle();
		ble_conn_params_change(param->ble_min_conn_interval,
							   param->ble_max_conn_interval,
							   param->ble_slave_latency,
							   param->ble_conn_timeout);
		
		SWT_t* timer = SWT_GetHandle();
		timer->BleAdvLed->Stop();
		timer->BleAdvLed->Start((param->ble_min_conn_interval+param->ble_max_conn_interval)/2);
	}
}

extern void LORA_Config(void);
extern void LORA_ConfigDefault(void);
extern void LORA_TaskStart(void);
static void lora_cfg_char_change_handler(void)
{
	if(lora_param_change == 0)
	{
		return;
	}	
	
	if(lora_param_change & LORA_FREQ_CHANGE)
	{
		lora_param_change &= ~LORA_FREQ_CHANGE;
	}
	
	if(lora_param_change & LORA_POWER_CHANGE)
	{
		lora_param_change &= ~LORA_POWER_CHANGE;
	}

	if(lora_param_change & LORA_BW_CHANGE)
	{
		lora_param_change &= ~LORA_BW_CHANGE;
	}

	if(lora_param_change & LORA_SF_CHANGE)
	{
		lora_param_change &= ~LORA_SF_CHANGE;
	}

	if(lora_param_change & LORA_CODE_RATE_CHANGE)
	{
		lora_param_change &= ~LORA_CODE_RATE_CHANGE;
	}

	if(lora_param_change & LORA_PREAMBLE_CHANGE)
	{
		lora_param_change &= ~LORA_PREAMBLE_CHANGE;
	}

	if(lora_param_change & LORA_HEADER_CHANGE)
	{
		lora_param_change &= ~LORA_HEADER_CHANGE;
	}

	if(lora_param_change & LORA_CRC_CHANGE)
	{
		lora_param_change &= ~LORA_CRC_CHANGE;
	}
	
	LORA_ConfigDefault();
	nrf_delay_ms(10);
	LORA_Config();
	LORA_TaskStart();
}

static void dev_cfg_char_change_handler(void)
{
	if(dev_param_change == 0)
	{
		return;
	}	
	
	if(dev_param_change & DEV_GATEWAY_ADDR_CHANGE)
	{
		dev_param_change &= ~DEV_GATEWAY_ADDR_CHANGE;
	}
	
	if(dev_param_change & DEV_LONG_ADDR_CHANGE)
	{
		dev_param_change &= ~DEV_LONG_ADDR_CHANGE;
	}

	if(dev_param_change & DEV_SHORT_ADDR_CHANGE)
	{
		dev_param_change &= ~DEV_SHORT_ADDR_CHANGE;
	}	
}

void ble_char_change_handler(void)
{
	ble_cfg_char_change_handler();
	lora_cfg_char_change_handler();
	dev_cfg_char_change_handler();
}













































