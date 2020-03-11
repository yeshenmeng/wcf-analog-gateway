#include "inclinometer.h"
#include <math.h>
#include <stdlib.h>
#include "sca100t.h"
#include "nrf_delay.h"


#define INCLINOMETER_PWOER_ENBALE()		nrf_gpio_pin_set(INCLINOMETER_PWOER_PIN)
#define INCLINOMETER_PWOER_DISABLE()	nrf_gpio_pin_clear(INCLINOMETER_PWOER_PIN)

/* SCA100T系列灵敏度 */
#define	SCA100T_D01_RESOLUTION				1638	
#define	SCA100T_D02_RESOLUTION				819
#define SCA100T_D01_DATA_VALID_MIN		205
#define SCA100T_D01_DATA_VALID_MAX		1843

#define SCA100T_READ_DELAY						1
uint32_t READ_DELAY = 5;
const uint16_t ScaResolution[] = {SCA100T_D01_RESOLUTION, SCA100T_D02_RESOLUTION};
static SCA_Type ScaType = SCA_D01;
static Inclinometer_Task_State InclinometerTaskStatus;
static Inclinometer_Info_t Inclinometer_Info;

__weak void Inclinometer_TaskStopHandler(void* param);

static float Inclinometer_ReadTemp(void)
{
	float tempDigital = (float)SCA_ReadTemperature();
	return (tempDigital - 197) / (-1.083);
}

static int Compare_Uint16(const void *data1, const void *data2)
{
	return *(uint16_t*)data1 - *(uint16_t*)data2;
}

//如果读取的数据小于1024(负数)则代表右倾即左摆，大于1024（正数）则代表左倾即右摆
//转换角度函数，45.45度为4545
float g_f_acceleration = 0;
float g_angle = 0;
static float Inclinometer_ReadXAngle(uint8_t ReadTimes)
{
//	SCA_WriteCommand(STX);
//	SCA_WriteCommand(MEAS);
	
	if(ReadTimes == 0)
	{
		return 0.0;
	}
	
	uint16_t acceleration;
	uint16_t* pValue = (uint16_t*)malloc(sizeof(uint16_t)*ReadTimes);
	
	for(int i=0; i<ReadTimes; i++)
	{
		acceleration  = (uint16_t)SCA_ReadXChannel();
		if(acceleration<SCA100T_D01_DATA_VALID_MIN || acceleration>SCA100T_D01_DATA_VALID_MAX)
		{
			if(acceleration<SCA100T_D01_DATA_VALID_MIN)
			{
				acceleration = SCA100T_D01_DATA_VALID_MIN;
			}
			else
			{
				acceleration = SCA100T_D01_DATA_VALID_MAX;
			}
		}
		*(pValue+i) = acceleration;
		nrf_delay_ms(READ_DELAY);
	}
	
	qsort(pValue, ReadTimes, sizeof(uint16_t), Compare_Uint16);

	uint32_t temp = 0;
	for(int i=1; i<ReadTimes-1; i++)
	{
		temp += pValue[i];
	}
	float f_acceleration = temp / (ReadTimes-2);
	g_f_acceleration = f_acceleration;
	free((void*)pValue);

	float angle = (asin((f_acceleration - 1024) / ScaResolution[ScaType])) * 180 /3.1415926;
	return angle;
}

uint16_t data_y[10];
uint16_t data_y_min = 0XFFFF;
uint16_t data_y_max = 0;
static float Inclinometer_ReadYAngle(uint8_t ReadTimes)
{
//	SCA_WriteCommand(STY);
//	SCA_WriteCommand(MEAS);
	
	if(ReadTimes == 0)
	{
		return 0.0;
	}
	
	uint16_t acceleration;
	uint16_t* pValue = (uint16_t*)malloc(sizeof(uint16_t)*ReadTimes);
	for(int i=0; i<ReadTimes; i++)
	{
		acceleration = SCA_ReadYChannel();
		data_y[i] = acceleration;
		if(data_y_min > acceleration) data_y_min = acceleration;
		if(data_y_max < acceleration) data_y_max = acceleration;		
		
		if(acceleration<SCA100T_D01_DATA_VALID_MIN || acceleration>SCA100T_D01_DATA_VALID_MAX)
		{
			if(acceleration<SCA100T_D01_DATA_VALID_MIN)
			{
				acceleration = SCA100T_D01_DATA_VALID_MIN;
			}
			else
			{
				acceleration = SCA100T_D01_DATA_VALID_MAX;
			}
		}
		*(pValue+i) = acceleration;
		nrf_delay_ms(READ_DELAY);
	}
	
	qsort(pValue, ReadTimes, sizeof(uint16_t), Compare_Uint16);
	qsort(data_y, ReadTimes, sizeof(uint16_t), Compare_Uint16);
	
	uint32_t temp = 0;
	for(int i=1; i<ReadTimes-1; i++)
	{
		temp += pValue[i];
	}
	float f_acceleration = temp / (ReadTimes-2);
	free((void*)pValue);

	float angle = (asin((f_acceleration - 1024) / ScaResolution[ScaType])) * 180 /3.1415926;
	return angle;
}

static void Inclinometer_TaskStart(void)
{
	if(Inclinometer_Info.State == INCLINOMETER_TASK_IDLE)
	{
		Inclinometer_Info.LPMHandle->TaskSetStatus(INCLINOMETER_TASK_ID, LPM_TASK_STA_RUN);
		InclinometerTaskStatus = INCLINOMETER_TASK_ACTIVE;
	}
	return;
}

static void Inclinometer_TaskStop(void)
{
	if(Inclinometer_Info.State != INCLINOMETER_TASK_IDLE)
	{
		InclinometerTaskStatus = INCLINOMETER_TASK_STOP;
	}
	return;	
}

uint32_t power_delay = 20;
static void Inclinometer_TaskOperate(void)
{
	Inclinometer_Task_State stateTmp;
	switch((uint8_t)InclinometerTaskStatus)
	{
		case INCLINOMETER_TASK_ACTIVE:
			SCA_Init();
			INCLINOMETER_PWOER_ENBALE(); //打开倾角采样功能
			nrf_delay_ms(power_delay);
			InclinometerTaskStatus = INCLINOMETER_TASK_IDLE;
			Inclinometer_Info.State = INCLINOMETER_TASK_ACTIVE;
			Inclinometer_Info.LPMHandle->TaskSetStatus(INCLINOMETER_TASK_ID, LPM_TASK_STA_RUN);
			Inclinometer_Info.Data.XAngle = Inclinometer_ReadXAngle(5);
			Inclinometer_Info.Data.YAngle = Inclinometer_ReadYAngle(5);
			Inclinometer_Info.Data.Temperature = Inclinometer_ReadTemp();
			Inclinometer_Info.Data.UpdateFlag = 1;
			InclinometerTaskStatus = INCLINOMETER_TASK_STOP;
			break;
		
		case INCLINOMETER_TASK_STOP:
			SCA_Default();
			INCLINOMETER_PWOER_DISABLE(); //关闭倾角采样功能
			stateTmp = InclinometerTaskStatus;
			InclinometerTaskStatus = INCLINOMETER_TASK_IDLE;
			Inclinometer_Info.State = INCLINOMETER_TASK_IDLE;
			Inclinometer_Info.LPMHandle->TaskSetStatus(INCLINOMETER_TASK_ID, LPM_TASK_STA_STOP);
			Inclinometer_TaskStopHandler((void *)&stateTmp); /* 倾角传感器任务停止处理 */
			break;
	}
	
	return;	
}

static void Inclinometer_Config(void)
{
	nrf_gpio_cfg(INCLINOMETER_PWOER_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);
	
	INCLINOMETER_PWOER_DISABLE();
}

Inclinometer_Info_t* Inclinometer_TaskInit(LPM_t* LPMHandle)
{
	ScaType = SCA_D01;
	Inclinometer_Config();
	SCA_Init();
	SCA_WriteCommand(STX);
	SCA_WriteCommand(STY);
	SCA_WriteCommand(MEAS);
	
	InclinometerTaskStatus = INCLINOMETER_TASK_IDLE;
	Inclinometer_Info.State = INCLINOMETER_TASK_IDLE;	
	
	Inclinometer_Info.Data.UpdateFlag = 0;
	Inclinometer_Info.Data.Temperature = 255;
	Inclinometer_Info.Data.XAngle = 255;
	Inclinometer_Info.Data.YAngle = 255;
	Inclinometer_Info.LPMHandle = LPMHandle;
	
	Inclinometer_Info.TaskStart = Inclinometer_TaskStart;
	Inclinometer_Info.TaskStop = Inclinometer_TaskStop;
	Inclinometer_Info.TaskOperate = Inclinometer_TaskOperate;
	Inclinometer_Info.ReadTemp = Inclinometer_ReadTemp;
	Inclinometer_Info.ReadXAngle = Inclinometer_ReadXAngle;
	Inclinometer_Info.ReadYAngle = Inclinometer_ReadYAngle;

	Inclinometer_Info.LPMHandle->TaskRegister(INCLINOMETER_TASK_ID);
	
	return &Inclinometer_Info;
}

/* 倾角传感器任务停止处理 */
__weak void Inclinometer_TaskStopHandler(void* param)
{
	return;
}





