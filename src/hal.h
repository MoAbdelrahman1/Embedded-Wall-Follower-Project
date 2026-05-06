#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include "stm32f401xc.h"

/* Simple blocking delay using SysTick */
void System_Init(void);
void HAL_Delay(uint32_t ms);

#endif