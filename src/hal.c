#include "hal.h"

static volatile uint32_t ms_ticks = 0;
void SysTick_Handler(void) { ms_ticks++; }

void System_Init(void)
{
    /* ── 1. Enable HSI, wait ready ────────────────────────── */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    /* ── 2. Flash latency: 2 WS for 84MHz ─────────────────── */
    FLASH->ACR = FLASH_ACR_PRFTEN
               | FLASH_ACR_ICEN
               | FLASH_ACR_DCEN
               | FLASH_ACR_LATENCY_2WS;
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2WS);

    /* ── 3. Configure PLL ──────────────────────────────────── 
       HSI = 16MHz
       VCO input  = 16 / M=16 = 1MHz
       VCO output = 1 x N=336 = 336MHz
       SYSCLK     = 336 / P=4 = 84MHz
       USB/SDIO   = 336 / Q=7 = 48MHz  */
    RCC->PLLCFGR = (16U  <<  0)               /* PLLM  */
                 | (336U <<  6)               /* PLLN  */
                 | (1U   << 16)               /* PLLP = 4 (0b01) */
                 | (7U   << 24)               /* PLLQ  */
                 | RCC_PLLCFGR_PLLSRC_HSI;

    /* ── 4. Enable PLL, wait ready ────────────────────────── */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* ── 5. APB1 = /2 (max 42MHz), APB2 = /1 (84MHz) ─────── */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    /* ── 6. Switch SYSCLK to PLL ───────────────────────────── */
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    /* ── 7. SysTick 1ms @ 84MHz ────────────────────────────── */
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

void TIM2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = 84 - 1;        /* 84MHz / 84 = 1MHz → 1µs per tick */
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->CNT = 0;
    TIM2->CR1 |= TIM_CR1_CEN;
}

// void delay_us(uint32_t us)
// {
//     uint32_t start = TIM2->CNT;
//     while ((TIM2->CNT - start) < us);
// }