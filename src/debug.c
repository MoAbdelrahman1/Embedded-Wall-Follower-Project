#include "debug.h"
#include "stm32f401xc.h"
#include <stdio.h>

void Debug_Init(uint32_t baud) {
    /* Enable GPIOA and USART1 clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9 (TX), PA10 (RX) -> AF7 (USART1) */
    GPIOA->MODER &= ~((3U << 18) | (3U << 20));
    GPIOA->MODER |=  ((2U << 18) | (2U << 20));
    GPIOA->AFR[1] &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->AFR[1] |=  ((7U << 4) | (7U << 8));

    /* USART1 config: 84MHz clock, 115200 baud -> BRR = 45.5625 -> 45:9 */
    if (baud == 115200) {
        USART1->BRR = (45U << 4) | 9U;
    } else if (baud == 9600) {
        USART1->BRR = (546U << 4) | 14U;
    }
    
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static void UART1_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (uint8_t)c;
}

void Debug_Printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    for (int i = 0; i < len; i++) {
        UART1_SendChar(buffer[i]);
    }
}
