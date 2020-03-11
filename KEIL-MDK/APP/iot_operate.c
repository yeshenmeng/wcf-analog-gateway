#include "iot_operate.h"
#include "sys_param.h"
#include "calendar.h"


IoT_dev_t IoT_dev;


void IoT_WriteSampleInterval(uint32_t value)
{
	IoT_dev.sensor->writePropFromBuf(SAMPLE_INTERVAL_ID, (uint8_t *)&value);
}

void IoT_WriteTimeStamp(uint32_t value)
{
	IoT_dev.sensor->writePropFromBuf(TIME_STAMP_ID, (uint8_t *)&value);
}

void IoT_WriteBatteryLevel(uint8_t value)
{
	IoT_dev.sensor->writePropFromBuf(BATTERY_LEVEL_ID, (uint8_t *)&value);
}

void IoT_WriteTemperature(uint32_t value)
{
	IoT_dev.sensor->writePropFromBuf(TEMPERATURE_ID, (uint8_t *)&value);
}

void IoT_WriteXAngle(uint32_t value)
{
	IoT_dev.sensor->writePropFromBuf(DATA_X_ANGLE_ID, (uint8_t *)&value);
}

void IoT_WriteYAngle(uint32_t value)
{
	IoT_dev.sensor->writePropFromBuf(DATA_Y_ANGLE_ID, (uint8_t *)&value);
}

void IoT_SetSampleInterval(uint32_t value)
{
	sys_param_t* param = Sys_ParamGetHandle();
	param->iot_sample_interval = value; //设置采样间隔
	param->update_flag = 1;
}

void IoT_SetTimeStamp(uint32_t value)
{
	Calendar_t* calendar = Calendar_GetHandle();
	calendar->SetTimeStamp(value); //设置时间戳
}

void IoT_Operate(void)
{
	if(IoT_dev.sensor->isPropChanged(SAMPLE_INTERVAL_ID))
	{
		uint32_t value;
		IoT_dev.sensor->readPropToBuf(SAMPLE_INTERVAL_ID, (uint8_t *)&value);
		IoT_dev.sensor->resetPropChangeFlag(SAMPLE_INTERVAL_ID);
		IoT_SetSampleInterval(value);
	}
	
	if(IoT_dev.sensor->isPropChanged(TIME_STAMP_ID))
	{
		uint32_t value;
		IoT_dev.sensor->readPropToBuf(TIME_STAMP_ID, (uint8_t *)&value);
		IoT_dev.sensor->resetPropChangeFlag(TIME_STAMP_ID);
		IoT_SetTimeStamp(value);
	}
	
	if(IoT_dev.inclinometer_info->Data.UpdateFlag == 1)
	{
		IoT_dev.inclinometer_info->Data.UpdateFlag = 0;
		IoT_WriteTemperature(IoT_dev.inclinometer_info->Data.Temperature);
		IoT_WriteXAngle(IoT_dev.inclinometer_info->Data.XAngle);
		IoT_WriteYAngle(IoT_dev.inclinometer_info->Data.YAngle);
	}
	
	if(IoT_dev.gas_gauge_flag == 1)
	{
		IoT_dev.gas_gauge_flag = 0;
		IoT_WriteBatteryLevel(IoT_dev.gas_gauge);
	}
}

IoT_dev_t * IoT_Init(iot_object_t *sensor, 
										 Inclinometer_Info_t *inclinometer_handle)
{
	IoT_dev.gas_gauge_flag = 0;
	IoT_dev.gas_gauge = 100;

	IoT_dev.sensor = sensor;
	IoT_dev.inclinometer_info = inclinometer_handle;
	
	IoT_WriteSampleInterval(60);
	IoT_dev.sensor->resetPropChangeFlag(SAMPLE_INTERVAL_ID);	
	
	IoT_WriteTimeStamp(0);
	IoT_dev.sensor->resetPropChangeFlag(TIME_STAMP_ID);	
	
	IoT_WriteBatteryLevel(IoT_dev.gas_gauge);
	IoT_dev.sensor->resetPropChangeFlag(BATTERY_LEVEL_ID);	
	
	IoT_WriteTemperature(25);
	IoT_dev.sensor->resetPropChangeFlag(TEMPERATURE_ID);	
	
	IoT_WriteXAngle(0);
	IoT_dev.sensor->resetPropChangeFlag(DATA_X_ANGLE_ID);	
	
	IoT_WriteYAngle(0);
	IoT_dev.sensor->resetPropChangeFlag(DATA_Y_ANGLE_ID);	
	
	IoT_dev.operate = IoT_Operate;
	
	return &IoT_dev;
}



