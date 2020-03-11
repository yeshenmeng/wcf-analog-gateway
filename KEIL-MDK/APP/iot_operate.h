#ifndef __IOT_OPERATE_H__
#define __IOT_OPERATE_H__
#include "main.h"
#include "iotobject.h"
#include "inclinometer.h"


#define SAMPLE_INTERVAL_ID				3
#define TIME_STAMP_ID							4
#define BATTERY_LEVEL_ID					5
#define TEMPERATURE_ID						6
#define DATA_X_ANGLE_ID 					7
#define DATA_Y_ANGLE_ID 					8

typedef struct {
	uint8_t gas_gauge_flag;
	uint8_t gas_gauge;

	iot_object_t *sensor;
	Inclinometer_Info_t *inclinometer_info;

	void (*operate)(void);
} IoT_dev_t;

IoT_dev_t * IoT_Init(iot_object_t *sensor, 
					 Inclinometer_Info_t *inclinometer_handle);

#endif
