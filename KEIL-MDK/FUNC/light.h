#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "main.h"


#if (SYS_HW_VERSION == SYS_HW_VERSION_V0_3_0)
#define LED_1_PIN					16 //ÂÌµÆ
#define LED_2_PIN					15 //ºìµÆ
#elif (SYS_HW_VERSION == SYS_HW_VERSION_V0_1_1)
#define LED_1_PIN					20 //ÂÌµÆ
#define LED_2_PIN					19 //ºìµÆ
#define LED_3_PIN					18 //À¶µÆ
#endif

#define LIGHT_1_ON()				nrf_gpio_pin_clear(LED_1_PIN)
#define LIGHT_1_OFF()				nrf_gpio_pin_set(LED_1_PIN)
#define LIGHT_1_TOGGLE()			nrf_gpio_pin_toggle(LED_1_PIN)
#define LIGHT_2_ON()				nrf_gpio_pin_clear(LED_2_PIN)
#define LIGHT_2_OFF()				nrf_gpio_pin_set(LED_2_PIN)
#define LIGHT_2_TOGGLE()			nrf_gpio_pin_toggle(LED_2_PIN)

#if (SYS_HW_VERSION == SYS_HW_VERSION_V0_1_1)
#define LIGHT_3_ON()				nrf_gpio_pin_clear(LED_3_PIN)
#define LIGHT_3_OFF()				nrf_gpio_pin_set(LED_3_PIN)
#define LIGHT_3_TOGGLE()			nrf_gpio_pin_toggle(LED_3_PIN)
#endif

#if (SYS_HW_VERSION == SYS_HW_VERSION_V0_3_0)
#define LIGHT_OFF()\
do{\
	LIGHT_1_OFF();\
	LIGHT_2_OFF();\
}while(0);

#define LIGHT_ON()\
do{\
	LIGHT_1_ON();\
	LIGHT_2_ON();\
}while(0);
#endif

#if (SYS_HW_VERSION == SYS_HW_VERSION_V0_1_1)
#define LIGHT_OFF()\
do{\
	LIGHT_1_OFF();\
	LIGHT_2_OFF();\
	LIGHT_3_OFF();\
}while(0);

#define LIGHT_ON()\
do{\
	LIGHT_1_ON();\
	LIGHT_2_ON();\
	LIGHT_3_ON();\
}while(0);
#endif

void Light_Init(void);

#endif




