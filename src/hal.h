#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include "stm32f401xc.h"

/* ─── System / SysTick ─────────────────────────────────── */
void System_Init(void);
void HAL_Delay(uint32_t ms);

/* ─── TIM2 microsecond timer ───────────────────────────── */
void     TIM2_Init(void);
// void     delay_us(uint32_t us);

/* ─── HC-SR04 ultrasonic sensor ────────────────────────── */
void     HCSR04_Init(void);
float    HCSR04_Read_cm(void);

/* ─── USART1 @ 9600 baud (PA9=TX, PA10=RX) ────────────── */
void     UART1_Init_9600(void);
void     UART1_WriteChar(char c);
void     UART1_WriteString(const char *s);
void     UART1_WriteU32(uint32_t v);

#endif /* HAL_H */