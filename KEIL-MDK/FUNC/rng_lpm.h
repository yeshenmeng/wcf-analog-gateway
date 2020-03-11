#ifndef __RNG_LPM_H__
#define __RNG_LPM_H__
#include "main.h"


#define RNG_RANDOM_UPPER			10000u
#define RNG_RANDOM_LOWER			1u

typedef struct {
	uint32_t RandomUpper;
	uint32_t RandomLower;
	
	void (*GenerateRandomNumber)(uint32_t* RandomBuf, uint32_t size);
}RNG_LPM_t;


void RNG_LPM_Init(void);
RNG_LPM_t* RNG_LPM_GetHandle(void);

#endif



