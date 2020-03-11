#include "sca100t.h"
#include "nrf_gpio.h"


#define SCA_SW_SPI_SCK_SET()			nrf_gpio_pin_set(SCA_SW_SPI_SCK_PIN)
#define SCA_SW_SPI_SCK_CLR()			nrf_gpio_pin_clear(SCA_SW_SPI_SCK_PIN)
#define SCA_SW_SPI_MOSI_SET()			nrf_gpio_pin_set(SCA_SW_SPI_MOSI_PIN)
#define SCA_SW_SPI_MOSI_CLR()			nrf_gpio_pin_clear(SCA_SW_SPI_MOSI_PIN)
#define SCA_SW_SPI_CS_ENABLE()		nrf_gpio_pin_clear(SCA_SW_SPI_CS_PIN)
#define SCA_SW_SPI_CS_DISABLE()		nrf_gpio_pin_set(SCA_SW_SPI_CS_PIN)
#define SCA_SW_SPI_MISO_READ() 		nrf_gpio_pin_read(SCA_SW_SPI_MISO_PIN)


static void SCA_DelayUs(uint32_t nus)
{
		uint16_t i=0;  
		while(nus--)
		{
			i=10;
			while(i--); 
		}
}

static void SCA_SW_SPI_Config(void)
{
	nrf_gpio_cfg(SCA_SW_SPI_SCK_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);
	
	nrf_gpio_cfg(SCA_SW_SPI_MOSI_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);

	nrf_gpio_cfg(SCA_SW_SPI_MISO_PIN,
							 NRF_GPIO_PIN_DIR_INPUT,
							 NRF_GPIO_PIN_INPUT_CONNECT,
//							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_PULLDOWN,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);

	nrf_gpio_cfg(SCA_SW_SPI_CS_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);
}

static void SCA_SW_SPI_ConfigDefault(void)
{
	nrf_gpio_cfg_default(SCA_SW_SPI_SCK_PIN);
	nrf_gpio_cfg_default(SCA_SW_SPI_MOSI_PIN);
	nrf_gpio_cfg_default(SCA_SW_SPI_MISO_PIN);
	nrf_gpio_cfg_default(SCA_SW_SPI_CS_PIN);
}

static void SCA_SW_SPI_TransmitReceive(uint16_t TxData, uint8_t TxBitSize, uint16_t *pRxData, uint8_t RxBitSize)
{
	SCA_SW_SPI_CS_ENABLE();
	
	uint16_t txData = TxData;
	uint16_t rxData = 0;
	
	if(TxBitSize != 0)
	{
		for(int i=0; i<TxBitSize; i++)
		{
			SCA_SW_SPI_SCK_CLR();
			if(txData & 0X80)
			{
				SCA_SW_SPI_MOSI_SET();
			}
			else
			{
				SCA_SW_SPI_MOSI_CLR();
			}
			txData <<= 1;
			SCA_DelayUs(2);
			SCA_SW_SPI_SCK_SET();
			SCA_DelayUs(2);
			
			rxData <<= 1;
			if(SCA_SW_SPI_MISO_READ())
			{
				rxData++;
			}
		}
		SCA_SW_SPI_SCK_CLR();
	}
	
	if(RxBitSize != 0)
	{
		rxData = 0;
		for(int i=0; i<RxBitSize; i++)
		{
			SCA_SW_SPI_SCK_CLR();
			SCA_DelayUs(2);
			SCA_SW_SPI_SCK_SET();
			SCA_DelayUs(2);
			rxData <<= 1;
			if(SCA_SW_SPI_MISO_READ())
			{
				rxData++;
			}
		}
	}

	if(pRxData != NULL)
	{
		*pRxData = rxData;
	}
	
	SCA_SW_SPI_SCK_CLR();
	SCA_SW_SPI_CS_DISABLE();
	
	return;
}

void SCA_Init(void)
{
	SCA_SW_SPI_Config();
}

void SCA_Default(void)
{
	SCA_SW_SPI_ConfigDefault();
}

void SCA_WriteCommand(uint8_t cmd)
{
	SCA_SW_SPI_TransmitReceive(cmd, 8, NULL, 0);
}

uint8_t SCA_ReadTemperature(void)
{
	uint16_t temp;
	SCA_SW_SPI_TransmitReceive(RWTR, 8, &temp, 8);
	return (uint8_t)temp;	
}

uint16_t SCA_ReadXChannel(void)
{
	uint16_t xChannelData;
	SCA_SW_SPI_TransmitReceive(RDAX, 8, &xChannelData, 11);
	return xChannelData;
}

uint16_t SCA_ReadYChannel(void)
{
	uint16_t yChannelData;
	SCA_SW_SPI_TransmitReceive(RDAY, 8, &yChannelData, 11);
	return yChannelData;	
}























