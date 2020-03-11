#include "rng_lpm.h"
#include "nrf_drv_rng.h"


RNG_LPM_t RNG_LPM_Info;


static void RNG_LPM_GenerateRandomNumber(uint32_t* RandomBuf, uint32_t size)
{
	uint32_t err_code;
	uint8_t  available;

	nrf_drv_rng_bytes_available(&available);
	uint8_t length = MIN(size*4, available);

	err_code = nrf_drv_rng_rand((uint8_t*)RandomBuf, length);
	APP_ERROR_CHECK(err_code);
	
	for(int32_t i=0; i<size; i++)
	{
		RandomBuf[i] = RandomBuf[i] % (RNG_LPM_Info.RandomUpper - RNG_LPM_Info.RandomLower) + RNG_LPM_Info.RandomLower;
	}

	return;
}

void RNG_LPM_Init(void)
{
	uint32_t err_code = nrf_drv_rng_init(NULL);
	APP_ERROR_CHECK(err_code);
	
	RNG_LPM_Info.RandomUpper = RNG_RANDOM_UPPER;
	RNG_LPM_Info.RandomLower = RNG_RANDOM_LOWER;
	
	RNG_LPM_Info.GenerateRandomNumber = RNG_LPM_GenerateRandomNumber;
}

RNG_LPM_t* RNG_LPM_GetHandle(void)
{
	return &RNG_LPM_Info;
}


__weak void RNG_ErrHandler(void)
{
	return;
}  









