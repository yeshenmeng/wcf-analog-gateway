#include "signal_detect.h"
#include "nrf_drv_gpiote.h"


__weak void Signal_DetEvtHandler(void* param);

static void Signal_DetConfig(void)
{
	nrf_gpio_cfg(SIGNAL_DET_PWOER_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);
	
	SIGNAL_DET_PWOER_DISABLE();
}

#ifdef SIGNAL_DET_USE_INT_COPM
static void LPCOMP_EvtHandler(nrf_lpcomp_event_t event)
{
	if (event == NRF_LPCOMP_EVENT_UP)
	{
		SIGNAL_DET_PWOER_DISABLE();
		Signal_DetEvtHandler(NULL);
	}
}

static void LPCOMP_Init(void)
{
	uint32_t err_code;
	nrf_drv_lpcomp_config_t config;
	
	config.hal.reference = NRF_LPCOMP_REF_SUPPLY_1_16;
	config.hal.detection = NRF_LPCOMP_DETECT_UP;
	config.hal.hyst = NRF_LPCOMP_HYST_NOHYST;
	config.input = SIGNAL_DET_AIN_INPUT;
	config.interrupt_priority = LPCOMP_CONFIG_IRQ_PRIORITY;
	// initialize LPCOMP driver, from this point LPCOMP will be active and provided
	// event handler will be executed when defined action is detected
	err_code = nrf_drv_lpcomp_init(&config, LPCOMP_EvtHandler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_lpcomp_enable();
}
#endif

#ifdef SIGNAL_DET_USE_EXT_COPM
static void gpiote_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(pin == SIGNAL_LINE_GPIO_PIN)
	{
		if(action == GPIOTE_CONFIG_POLARITY_LoToHi)
		{
			SIGNAL_DET_PWOER_DISABLE();
			Signal_DetEvtHandler(NULL);
		}
	}
}

static void Signal_ExtIntConfig(void)
{
	nrfx_gpiote_in_config_t nrfx_gpiote_in_config;
	
	if (!nrf_drv_gpiote_is_init())
	{
			uint32_t err_code;
			err_code = nrf_drv_gpiote_init();
			APP_ERROR_CHECK(err_code);
	}	
	
	nrfx_gpiote_in_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;
	nrfx_gpiote_in_config.is_watcher = false;
	nrfx_gpiote_in_config.pull = NRF_GPIO_PIN_NOPULL;
	nrfx_gpiote_in_config.hi_accuracy = true;
	nrf_drv_gpiote_in_init(SIGNAL_LINE_GPIO_PIN,
												 &nrfx_gpiote_in_config,
												 &gpiote_in_pin_handler);
	
	nrf_drv_gpiote_in_event_enable(SIGNAL_LINE_GPIO_PIN, true);
}
#endif

void Signal_DetInit(void)
{
	Signal_DetConfig();
	
#ifdef SIGNAL_DET_USE_INT_COPM
	LPCOMP_Init();
#endif
	
#ifdef SIGNAL_DET_USE_EXT_COPM
	Signal_ExtIntConfig();
#endif
}

/* 信号检测中断事件处理 */
__weak void Signal_DetEvtHandler(void* param)
{
	return;
}




