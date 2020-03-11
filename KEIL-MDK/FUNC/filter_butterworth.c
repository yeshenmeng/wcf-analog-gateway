#include "filter_butterworth.h"

#define FILTER_ORDER_N		6			//过滤器阶数

const double b[] = {					//常系数
	0.000000332396951,
	0.000001994381709,
	0.000004985954273,
	0.000006647939031,
	0.000004985954273,
	0.000001994381709,
	0.000000332396951,
};

const double a[] = {
	-5.321277971919750,
	11.832899312675250,
	-14.071792433028330,
	9.437123969899679,
	-3.383547633321229,
	0.506616029099276,
};

static uint32_t filter_nums = 0;
static float x_buf[FILTER_ORDER_N+1] = {0};
static float y_buf[FILTER_ORDER_N+1] = {0};

float filter_butter(float value, uint8_t reset_flag)
{
	uint8_t n;
	
	if(reset_flag == 1)
	{
		filter_nums = 0;
	}
	
	(++filter_nums > FILTER_ORDER_N) ? (n = FILTER_ORDER_N + 1) : (n = filter_nums);
	
	if(filter_nums > (FILTER_ORDER_N + 1))
	{
		for(int i = 0; i < FILTER_ORDER_N; i++)
		{
			x_buf[i] = x_buf[i+1];
			y_buf[i] = y_buf[i+1];
		}
		
		x_buf[FILTER_ORDER_N] = value;
	}
	else
	{
		x_buf[filter_nums-1] = value;
	}
	
	double x_sum = 0;
	for(int i = 0; i < n; i++)
	{
		x_sum += b[i] * x_buf[n-i-1];
	}
	
	double y_sum = 0;
	for(int i = 0; i < (n-1); i++)
	{
		y_sum += a[i] * y_buf[n-i-2];
	}

	return y_buf[n-1] = x_sum - y_sum;
}

