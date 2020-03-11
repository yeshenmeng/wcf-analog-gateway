#ifndef __SIGNAL_DETECT_H__
#define __SIGNAL_DETECT_H__
#include "main.h"
#include "nrf_drv_lpcomp.h"


#define SIGNAL_DET_USE_INT_COPM //使用内部比较器检测信号
//#define SIGNAL_DET_USE_EXT_COPM //使用外部比较器检测信号

#ifdef SIGNAL_DET_USE_INT_COPM
	#undef SIGNAL_DET_USE_EXT_COPM
#elif defined(SIGNAL_DET_USE_EXT_COPM)
	#undef SIGNAL_DET_USE_INT_COPM
#endif


#ifdef SIGNAL_DET_USE_EXT_COPM
	#define SIGNAL_LINE_GPIO_PIN						19
	#define SIGNAL_LINE_GPIO_PORT						P0
	#define SIGNAL_DETECT_EXTI_IRQHandler		NULL
#endif

#ifdef SIGNAL_DET_USE_INT_COPM					
	#define SIGNAL_DET_AIN_INPUT						NRF_LPCOMP_INPUT_5
#endif

#define SIGNAL_DET_PWOER_PIN							22
#define SIGNAL_DET_PWOER_PORT							P0
#define SIGNAL_DET_PWOER_ENBALE()					nrf_gpio_pin_set(SIGNAL_DET_PWOER_PIN)
#define SIGNAL_DET_PWOER_DISABLE()				nrf_gpio_pin_clear(SIGNAL_DET_PWOER_PIN)


void Signal_DetInit(void);

#endif


