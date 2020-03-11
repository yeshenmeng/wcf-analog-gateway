#ifndef __LORA_TRANSMISSION_H__
#define __LORA_TRANSMISSION_H__
#include "main.h"
#include "low_power_manage.h"


#define LORA_TASK_ID							1 //LORA����ID

#define SWT_LORA_TIME_SLICE_TIME				2500000*1000u //LORA���ݷ���ʱ��Ƭʱ��
#define SWT_LORA_TIMEOUT_TIME					3000u //LORA���ݷ��ͳ�ʱʱ��

#define LORA_DELAY_BASE_TIME					100u //LORA���ݷ���ʧ����ʱ����
#define LORA_RANDOME_DELAY_UPPER				100u //LORA���ݷ���ʧ�������ʱʱ������
#define LORA_RANDOME_DELAY_LOWER				10u //LORA���ݷ���ʧ�������ʱʱ������
#define LORA_TX_MAX_DELAY_TIME					1*1000u //LORA���ݷ���ʧ�������ʱʱ��
#define LORA_TX_MAX_FIAL_TIMES					~0u //LORA���ݷ���ʧ��������

/* LORAͨ��״̬ */
#define LORA_OUT_STATE_OFFLINE					0X01 //δ����
#define LORA_OUT_STATE_DISCON					0X02 //������̬�Ͽ�
#define LORA_OUT_STATE_LINK						0X03 //��δ���ӵ�����̬
#define LORA_OUT_STATE_CONNECT					0X04 //����̬

#if (SYS_HW_VERSION == SYS_HW_VERSION_V0_3_0)
/* LORAģ���������� */
#define LORA_POWER_PIN							4 //LORAģ�鹩������
#define LORA_TRANSMIT_PIN						18 //LORA����
#define LORA_RECEIVE_PIN						17 //LORA����
#define LORA_RESET_PIN							25 //LORA��λ����
#define LORA_BUSY_PIN							26 //LORA BUSY����
#define LORA_IRQ_PIN							27 //LORA�ж�����

/* LORA_SPI�������� */
#define LORA_SPI_CS_PIN							5 //LORA_SPIƬѡ����
#define LORA_SPI_SCK_PIN    					6 //LORA_SPIʱ������
#define LORA_SPI_MISO_PIN   					8 //LORA_SPI����ӳ�����
#define LORA_SPI_MOSI_PIN   					7 //LORA_SPI������������
#elif (SYS_HW_VERSION == SYS_HW_VERSION_V0_1_1)
/* LORAģ���������� */
#define LORA_POWER_PIN							9 //LORAģ�鹩������
#define LORA_TRANSMIT_PIN						2 //LORA����
#define LORA_RECEIVE_PIN						3 //LORA����
#define LORA_RESET_PIN							8 //LORA��λ����
#define LORA_BUSY_PIN							30 //LORA BUSY����
#define LORA_IRQ_PIN							31 //LORA�ж�����

/* LORA_SPI�������� */
#define LORA_SPI_CS_PIN							4 //LORA_SPIƬѡ����
#define LORA_SPI_SCK_PIN    					5 //LORA_SPIʱ������
#define LORA_SPI_MISO_PIN   					7 //LORA_SPI����ӳ�����
#define LORA_SPI_MOSI_PIN   					6 //LORA_SPI������������
#endif

/* LORA���Ź���״̬�궨�� */
#define LORA_PWOER_ENABLE()						nrf_gpio_pin_set(LORA_POWER_PIN) //ʹ��LORAģ��
#define LORA_PWOER_DISABLE()					nrf_gpio_pin_clear(LORA_POWER_PIN) //ʧ��LORAģ��
#define LORA_TRANSMIT_ENABLE()					nrf_gpio_pin_set(LORA_TRANSMIT_PIN) //ʹ��LORA����
#define LORA_TRANSMIT_DISABLE()					nrf_gpio_pin_clear(LORA_TRANSMIT_PIN) //ʧ��LORA����
#define LORA_RECEIVE_ENABLE()					nrf_gpio_pin_set(LORA_RECEIVE_PIN) //ʹ��LORA����
#define LORA_RECEIVE_DISABLE()					nrf_gpio_pin_clear(LORA_RECEIVE_PIN) //ʧ��LORA����
#define LORA_RESET_ENABLE()						nrf_gpio_pin_clear(LORA_RESET_PIN) //ʹ��LORA��λ
#define LORA_RESET_DISABLE()					nrf_gpio_pin_set(LORA_RESET_PIN) //ʧ��LORA��λ
#define LORA_READ_STATUS()						nrf_gpio_pin_read(LORA_BUSY_PIN) //��ȡLORAæµ״̬
#define LORA_READ_IRQ_STATUS()					nrf_gpio_pin_read(LORA_IRQ_PIN) //��ȡ�ж�����״̬
#define LORA_SPI_CS_ENABLE()					nrf_gpio_pin_clear(LORA_SPI_CS_PIN) //LORA SPIƬѡʹ��
#define LORA_SPI_CS_DISABLE()					nrf_gpio_pin_set(LORA_SPI_CS_PIN) //LORA SPIƬѡʧ��

#define LONG_ADDR				0X01
#define SHORT_ADDR				0X02
#define SAMPLE_TIME_CTR			0X04
#define FIXED_POINT_TIME		0X08
#define TIME_STAMP				0X10
#define TIME_OFFSET				0X20
#define GATEWAY_ADDR			0X40
#define PRINT_CTRL				0X80
#define DEV_CTRL				0X100
#define SAMPLE_PERIOD			0X200
#define HEART_BEAT_PERIOD		0X400
#define SENS_FREQ				0X800
#define SUM_FRAGMENT			0X1000
#define FRAGMENT_SIZE			0X2000
#define TRANSMIT_FLAG			0X4000


#define PRINT_LORA_REPLY_MSG	0

typedef enum {
	SENS_FREQ_4HZ = 0,
	SENS_FREQ_8HZ,
	SENS_FREQ_16HZ,
	SENS_FREQ_31HZ,
	SENS_FREQ_62HZ,
	SENS_FREQ_125HZ,
	SENS_FREQ_250HZ,
	SENS_FREQ_500HZ,
	SENS_FREQ_1000HZ,
	SENS_FREQ_2000HZ,
	SENS_FREQ_4000HZ,
}sens_freq_t;

typedef struct {
	uint32_t status;
	uint8_t long_addr[8];
	uint8_t gateway_addr[8];
	uint8_t short_addr[2];
	uint8_t sample_time_ctr;
	uint32_t fixed_point_time;
	uint32_t interval;
	uint32_t time_stamp;
	uint16_t time_offset;
	uint32_t sample_period;
	uint16_t heart_beat_period;
	sens_freq_t sens_freq;
	uint8_t sum_fragment;
	uint16_t fragment_size;
	uint8_t transmit_flag;
	float x_thres;
	float y_thres;
	float z_thres;
}lora_reply_data_t;

typedef enum {
	LORA_IDLE,
	LORA_ACTIVE,
	LORA_TX_SUCCESS,
	LORA_TX_FAIL,
	LORA_TIMEOUT,
	LORA_STOP,
}Lora_State;

typedef struct {
	uint32_t TaskTimeSlice;
	uint32_t TxTimeout;
	uint32_t TxMaxFailTimes;
	
	uint32_t DelayBaseTime;
	uint32_t RandomDelayUpper;
	uint32_t RandomDelayLower;
	uint32_t TxMaxDelayTime;
	
	/* ���Ʋ��� */
	FunctionalState IsIdleEnterLp; //DISABLE��LORA����״̬��������ENABLE��LORA����״̬����͹���ģʽ
	
	/* �������� */
	uint32_t TxFailTimes;
}Lora_Param_t;

typedef struct {
	Lora_Param_t Param;
	Lora_State State;
	
	LPM_t* LPMHandle;
	
	void (*TaskStart)(void);
	void (*TaskStop)(void);
	void (*SetTxData)(uint8_t* pData, uint8_t size);
	void (*StatusProc)(void);
}Lora_Info_t;

Lora_Info_t* LORA_TaskInit(LPM_t* LPMHandle);
void LORA_SPI_Transfer(uint8_t* tx_buffer, uint8_t tx_length, uint8_t* rx_buffer, uint8_t rx_length);

#endif


