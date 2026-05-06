#include "ultrasonic.h"
#include "config.h"
#include "stm32f401xc.h"

/* We use TIM3 as a free-running 1MHz counter (1 tick = 1 µs) */

void Ultrasonic_Init(void)
{
    /* Enable GPIOB and TIM3 clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;

    /* PB0 (TRIG) → output push-pull */
    GPIOB->MODER  &= ~(3U << (TRIG_PIN * 2));
    GPIOB->MODER  |=  (1U << (TRIG_PIN * 2));
    GPIOB->OTYPER &= ~(1U << TRIG_PIN);
    GPIOB->BSRR    =  (1U << (TRIG_PIN + 16)); /* TRIG starts LOW */

    /* PB1 (ECHO) → input, no pull */
    GPIOB->MODER  &= ~(3U << (ECHO_PIN * 2));  /* 00 = input */
    GPIOB->PUPDR  &= ~(3U << (ECHO_PIN * 2));  /* no pull */

    /* TIM3: free-running at 1 MHz (1 tick = 1 µs) */
    TIM3->PSC = 83U;        /* 84MHz / 84 = 1MHz */
    TIM3->ARR = 0xFFFFU;    /* max count */
    TIM3->EGR = TIM_EGR_UG;
    TIM3->CR1 = TIM_CR1_CEN;
}

/* Blocking µs delay using TIM3 */
static void delay_us(uint32_t us)
{
    uint32_t start = TIM3->CNT;
    while ((TIM3->CNT - start) < us);
}

uint32_t Ultrasonic_Read_cm(void)
{
    uint32_t t_start, t_end, elapsed;

    /* Send 10µs TRIG pulse */
    GPIOB->BSRR = (1U << TRIG_PIN);        /* TRIG high */
    delay_us(10);
    GPIOB->BSRR = (1U << (TRIG_PIN + 16)); /* TRIG low  */

    /* Wait for ECHO to go HIGH (with timeout) */
    t_start = TIM3->CNT;
    while (!(GPIOB->IDR & (1U << ECHO_PIN))) {
        if ((TIM3->CNT - t_start) > 30000U) return 999U; /* timeout */
    }

    /* Measure how long ECHO stays HIGH */
    t_start = TIM3->CNT;
    while (GPIOB->IDR & (1U << ECHO_PIN)) {
        if ((TIM3->CNT - t_start) > 30000U) return 999U; /* timeout */
    }
    t_end = TIM3->CNT;

    elapsed = t_end - t_start; /* in µs */

    return elapsed / 58U;      /* convert to cm */
}