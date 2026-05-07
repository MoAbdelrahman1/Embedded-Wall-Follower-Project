#include "hal.h"

static volatile uint32_t ms_ticks = 0;

void SysTick_Handler(void)
{
    ms_ticks++;
}

void System_Init(void)
{
    /* SysTick every 1ms, assuming 84MHz clock */
    SysTick_Config(84000U);
}

void HAL_Delay(uint32_t ms)
{
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms);
}

uint32_t HAL_GetTick(void)
{
    return ms_ticks;
}