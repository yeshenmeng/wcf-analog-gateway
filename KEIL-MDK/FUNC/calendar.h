#ifndef __CALENDAR_H__
#define __CALENDAR_H__
#include "main.h"


#define	RTC_SECONED_INT_EN			0 /* RTC秒中断使能 */

typedef unsigned int time_t;
 
typedef struct{
	int Second; /* 秒 – 取值区间为[0,59] */
	int Minute; /* 分 - 取值区间为[0,59] */
	int Hour; /* 时 - 取值区间为[0,23] */
	int Day; /* 一个月中的日期 - 取值区间为[1,31] */
	int Month; /* 月份 - 取值区间为[1,12] */
	int Year; /* 年份 - 取值区间为[1970.1.1,。。。] */
}Date_t;

typedef struct{
	Date_t* Date;
	
	uint32_t (*GetTicks)(void);
	time_t (*GetTimeStamp)(void);
	void (*SetTimeStamp)(time_t TimeStamp);
}Calendar_t;

void Calendar_TimeStampToDate(time_t time, Date_t* date);
time_t Calendar_DateToTimeStamp(Date_t* date);

void Calendar_Init(void);
Calendar_t* Calendar_GetHandle(void);

#endif









