#include "ultrasonic.h"
#include "stm32f401xc.h"

/* TIM3 = free-running 1MHz counter (1 tick = 1 µs) */

void Ultrasonic_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;

    TIM3->PSC = 83U;       /* 84MHz / 84 = 1MHz */
    TIM3->ARR = 0xFFFFU;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->CR1 = TIM_CR1_CEN;
}

static void delay_us(uint32_t us)
{
    uint32_t start = TIM3->CNT;
    while ((TIM3->CNT - start) < us);
}

uint32_t Ultrasonic_Read_cm(uint8_t trig_pin, uint8_t echo_pin)
{
    uint32_t t_start, t_end;

    /* Configure TRIG as output */
    GPIOB->MODER &= ~(3U << (trig_pin * 2));
    GPIOB->MODER |=  (1U << (trig_pin * 2));
    GPIOB->OTYPER &= ~(1U << trig_pin);

    /* Configure ECHO as input */
    GPIOB->MODER &= ~(3U << (echo_pin * 2));
    GPIOB->PUPDR &= ~(3U << (echo_pin * 2));

    /* Send 10µs TRIG pulse */
    GPIOB->BSRR = (1U << trig_pin);
    delay_us(10);
    GPIOB->BSRR = (1U << (trig_pin + 16));

    /* Wait for ECHO to go HIGH (timeout 30ms) */
    t_start = TIM3->CNT;
    while (!(GPIOB->IDR & (1U << echo_pin))) {
        if ((TIM3->CNT - t_start) > 30000U) return 999U;
    }

    /* Measure how long ECHO stays HIGH */
    t_start = TIM3->CNT;
    while (GPIOB->IDR & (1U << echo_pin)) {
        if ((TIM3->CNT - t_start) > 30000U) return 999U;
    }
    t_end = TIM3->CNT;

    return (t_end - t_start) / 58U; /* µs -> cm */
}