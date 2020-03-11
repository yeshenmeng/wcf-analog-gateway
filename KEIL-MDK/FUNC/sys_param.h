#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__
#include "main.h"

#define SYS_SW_MAIN_VERSION					0X01 //����汾
#define SYS_SW_SUB_VERSION					0X01
#define SYS_SW_MODIFY_VERSION				(uint16_t)0X0001	

#define SYS_HW_MAIN_VERSION					0X01 //Ӳ���汾
#define SYS_HW_SUB_VERSION					0X01
#define SYS_HW_MODIFY_VERSION				(uint16_t)0X0001

#define SYS_PARAM_FLASH_PAGE_ADDR			ADDR_FLASH_PAGE_80 //ϵͳ�����洢��ַ

/* �豸���ָ�ʽ */
#define DEV_MAME_FORMAT						1	//0��ʹ�ò�㳤��ַ��Ϊ�豸���֣�1��ʹ���Զ��������Ϊ�豸����

#define SYS_PARAM_BLE_DEV_NAME				"GW_IN_2019_" //�豸����
#define SYS_PARAM_BLE_TX_POWER				6 //�������书��[0:-40dBm,1:-20dBm,2:-16dBm,3:-12dBm,4:-8dBm,5:-4dBm,6:0dBm,7:+3dBm,8:+4dBm]
#define SYS_PARAM_BLE_ADV_INTERVAL			100 //�����㲥���[20~10240ms]
#define SYS_PARAM_BLE_ADV_TIME				15 //�����㲥ʱ��[1~65535s,0:���޹㲥]
#define SYS_PARAM_BLE_MIN_CONN_INTERVAL		100 //����������С���[15~4000ms]
#define SYS_PARAM_BLE_MAX_CONN_INTERVAL		100 //�������������[15~4000ms]
#define SYS_PARAM_BLE_SLAVE_LATENCY			0 //�����ӻ����Ժ��Ե��¼���[0~499]
#define SYS_PARAM_BLE_CONN_TIMEOUT			4000 //�������ӳ�ʱʱ�䣬��ʱʱ������Ӧ���ڴӻ���ʱ*���������[100~32000ms]

#define SYS_PARAM_LORA_FREQ					470 //LORAƵ��[410~800MHz]
#define SYS_PARAM_LORA_POWER				(int)20 //LORA���书��[-9~22]
#define SYS_PARAM_LORA_BW					8 /* LORA����[0:7.81KHz ,1:10.24KHz,2:15.63KHz,3:20.83KHz,
												 4:31.25KHz,5:41.67KHz,6:62.50KHz,7:125KHz,8:250KHz,9:500KHz] */
#define SYS_PARAM_LORA_SF					7 //LORA��Ƶ����[5~12]
#define SYS_PARAM_LORA_CODE_RATE			1 //LORA������[1:CR4_5,2:CR4_6,3:CR4_7,4:CR4_8]
#define SYS_PARAM_LORA_PREAMBLE				14 //LORAǰ����[5~255]
#define SYS_PARAM_LORA_HEADER				0 //LORA��ͷ,SFΪ6ʱֻ��ʹ����ʽ��ͷ[0:��ʽ��ͷ,1:��ʽ��ͷ]
#define SYS_PARAM_LORA_CRC					1 //LORAУ��[0:��,1:��]

#define SYS_PARAM_DEV_GATEWAY_ADDR			{0x64,0x02,0X20,0X19,0X09,0X16,0X00,0X01} //���ص�ַ
#define SYS_PARAM_DEV_LONG_ADDR				{0XAA,0XAA,0XAA,0XAA,0X20,0X19,0X11,0X11} //��㳤��ַ
#define SYS_PARAM_DEV_SHORT_ADDR			{1,1} //���̵�ַ

#define SYS_PARAM_IOT_MODE					0 //���ݲ���ģʽ[0:����ģʽ,1:�����ֵģʽ,2:������ֵģʽ]
#define SYS_PARAM_IOT_SAMPLE_INTERVAL		8 //3600 //���ݲ����������λs
#define SYS_PARAM_IOT_X_ANGLE_THRESHOLD		(float)8.8 //��ֵģʽ�µ�X��Ƕ���ֵ
#define SYS_PARAM_IOT_Y_ANGLE_THRESHOLD		(float)6.6 //��ֵģʽ�µ�Y��Ƕ���ֵ

typedef struct {
	uint8_t update_flag;
	
	uint8_t ble_tx_power;
	uint16_t ble_adv_interval;
	uint16_t ble_adv_time;
	uint16_t ble_min_conn_interval;
	uint16_t ble_max_conn_interval;
	uint16_t ble_slave_latency;
	uint16_t ble_conn_timeout;
	
	uint16_t lora_freq;
	uint8_t lora_power;
	uint8_t lora_bw;
	uint8_t lora_sf;
	uint8_t lora_code_rate;
	uint8_t lora_preamble;
	uint8_t lora_header;
	uint8_t lora_crc;
	
	uint8_t dev_gateway_addr[8];
	uint8_t dev_long_addr[8];
	uint8_t dev_short_addr[2];
	
	uint8_t iot_mode;
	uint32_t iot_sample_interval;
	float iot_x_angle_threshold;
	float iot_y_angle_threshold;
	
	uint8_t (*saveParamToFlash)(void);
} sys_param_t;

void Sys_ParamInit(void);
uint8_t Sys_SaveParamToFlash(void);
sys_param_t* Sys_ParamGetHandle(void);
void sys_param_set(uint8_t* param, uint8_t* value, uint8_t len);


#endif








