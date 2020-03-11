#include "calendar.h"
#include "time.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "nrfx_rtc.h"
#include "string.h"


static Calendar_t m_calendar;
static Date_t m_time;
static uint8_t m_time_count_1s = 0;
volatile time_t m_timestamp = 0;

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC2. */
const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#if RTC_SECONED_INT_EN != 1	
volatile uint32_t m_ticks = 0;
volatile uint32_t m_pre_count = 0;
static uint32_t m_timestamp_base = 59*60;
//static uint32_t m_timestamp_base = 0;
#endif


/* time:0->1970-1-1-8:0:0 */
void Calendar_TimeStampToDate(time_t time, Date_t* date)
{
	unsigned int Pass4year;
	int hours_per_year;

	/* 东八区时间比格林尼治时间快8个小时 */
	time += (3600 * 8);
	
	date->Second=(int)(time % 60); //取秒时间
	time /= 60;
	
	date->Minute=(int)(time % 60); //取分钟时间
	time /= 60;
	
	Pass4year=((unsigned int)time / (1461L * 24L)); //取过去多少个四年，每四年有 1461*24 小时
	date->Year=(Pass4year << 2) + 1970; //计算年份
	time %= 1461L * 24L; //四年中剩下的小时数
	
	//校正闰年影响的年份，计算一年中剩下的小时数
	for (;;)
	{
		hours_per_year = 365 * 24; //一年的小时数
		
		//判断闰年
		if ((date->Year & 3) == 0)
		{
			hours_per_year += 24; //是闰年，一年则多24小时，即一天
		}
		
		if (time < hours_per_year)
		{
			break;
		}
		
		date->Year++;
		time -= hours_per_year;
	}
	
	date->Hour=(int)(time % 24); //小时数
	
	time /= 24; //一年中剩下的天数
	time++; //假定为闰年
	
	//校正闰年的误差，计算月份，日期
	if((date->Year & 3) == 0)
	{
		if (time > 60)
		{
			time--;
		}
		else
		{
			if (time == 60)
			{
				date->Month = 1;
				date->Day = 29;
				return ;
			}
		}
	}
	
	//计算月日
	for (date->Month = 0; Days[date->Month] < time;date->Month++)
	{
		time -= Days[date->Month];
	}
	
	date->Month += 1;
	date->Day = (int)(time);
 
	return;
}

static time_t mon_yday[2][12] =
{
	{0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
	{0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};


int IsLeap(int year)
{
	return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

/* 1970-1-1-8:0:0->time:0 */
time_t Calendar_DateToTimeStamp(Date_t* date)
{
	time_t result;
	int i =0;
	
	//以平年时间计算的秒数
	result = (date->Year - 1970) * 365 * 24 * 3600 +
			 (mon_yday[IsLeap(date->Year)][date->Month-1] + date->Day - 1) * 24 * 3600 +
			  date->Hour * 3600 + date->Minute * 60 + date->Second;
	
	for(i=1970; i < date->Year; i++) // 加上闰年的秒数
	{
		if(IsLeap(i))
		{
			result += 24 * 3600;
		}
	}
	
	/* 东八区时间比格林尼治时间快8个小时 */
	result -= (3600 * 8);
	
	return(result);
}

/** @brief: Function for handling the RTC2 interrupts.
 * Triggered on TICK and COMPARE0 match.
 */
static void Calendar_RTCHandler(nrf_drv_rtc_int_type_t int_type)
{
	if(int_type == NRF_DRV_RTC_INT_COMPARE0)
	{
	}
	else if (int_type == NRF_DRV_RTC_INT_TICK)
	{
		if(m_time_count_1s >=7 )//?D???μ?ê125ms￡??ùò?125*8 =1000s
		{
				m_time_count_1s = 0;
				m_timestamp++;
				Calendar_TimeStampToDate(m_timestamp, &m_time);
		}
		else
		{
				m_time_count_1s++;
		}
	}
}

#if RTC_SECONED_INT_EN != 1
#define MAX_RTC2_COUNTER_VAL     0x00FFFFFF 
static uint32_t ticks_elapse_get(void)
{
	uint32_t count = NRF_RTC2->COUNTER & MAX_RTC2_COUNTER_VAL;
	uint32_t elapse = count - m_pre_count;
	m_pre_count = count;
	if(count > 0X007FFFFF)
	{
		m_pre_count = 0;
		nrf_drv_rtc_counter_clear(&rtc);
	}
	return elapse;
}
#endif

#if RTC_SECONED_INT_EN == 1	
static time_t Calendar_GetTimeStamp(void)
{
	return m_timestamp;
}
#else
time_t Calendar_GetTimeStamp(void)
{
	m_ticks += ticks_elapse_get();
	m_timestamp = m_timestamp_base + m_ticks / 8;
	Calendar_TimeStampToDate(m_timestamp, &m_time);
	return m_timestamp;
}
#endif

#if RTC_SECONED_INT_EN == 1	
static void Calendar_SetTimeStamp(time_t TimeStamp)
{
	m_timestamp = TimeStamp;
}
#else
static void Calendar_SetTimeStamp(time_t TimeStamp)
{
	m_ticks = 0;
	m_pre_count = 0;
	nrf_drv_rtc_counter_clear(&rtc);
	m_timestamp_base = TimeStamp;
}
#endif

/** @brief Function initialization and configuration of RTC driver instance.
 */
static void Calendar_RTC2Config(void)
{
	uint32_t err_code;
			
	//Initialize RTC instance
	nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
	config.prescaler = 4095; 
	
	err_code = nrf_drv_rtc_init(&rtc, &config, Calendar_RTCHandler);
	APP_ERROR_CHECK(err_code);

#if RTC_SECONED_INT_EN == 1	
	//Enable tick event & interrupt
	nrf_drv_rtc_tick_enable(&rtc, true);
#endif
	
	//Power on RTC instance
	nrf_drv_rtc_enable(&rtc);
}

void Calendar_Init(void)
{
	Calendar_RTC2Config();
	
	m_calendar.Date = &m_time;
	m_calendar.GetTimeStamp = Calendar_GetTimeStamp;
	m_calendar.SetTimeStamp = Calendar_SetTimeStamp;
}

Calendar_t* Calendar_GetHandle(void)
{
	return &m_calendar;
}



