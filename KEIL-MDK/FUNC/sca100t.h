#ifndef __SCA100T_H__
#define __SCA100T_H__
#include "main.h"


/* ���ģ��SPI�������� */
#define SCA_SW_SPI_SCK_PIN				12
#define SCA_SW_SPI_MOSI_PIN				14
#define SCA_SW_SPI_MISO_PIN				13
#define SCA_SW_SPI_CS_PIN					11

/* ���ģ��SPI�˿����� */
#define SCA_SW_SPI_SCK_PORT				P0
#define SCA_SW_SPI_MOSI_PORT			P0
#define SCA_SW_SPI_MISO_PORT			P0
#define SCA_SW_SPI_CS_PORT				P0


//========SCA100T��������
#define MEAS  0x00	//����ģʽ
#define RWTR  0x08	//��д�¶����ݼĴ���
#define RDSR  0x0A	//״̬�Ĵ���
#define RLOAD 0x0B	//��������NV���ݵ��ڴ�����Ĵ���
#define STX   0x0E	//Xͨ���Լ�
#define STY   0x0F	//Yͨ���Լ�
#define RDAX  0x10	//��Xͨ�����ٶ�����
#define RDAY  0x11	//��Yͨ�����ٶ����� 


void SCA_Init(void);
void SCA_Default(void);
void SCA_WriteCommand(uint8_t cmd);
uint8_t SCA_ReadTemperature(void);
uint16_t SCA_ReadXChannel(void);
uint16_t SCA_ReadYChannel(void);


#endif


