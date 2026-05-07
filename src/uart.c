#include "uart.h"
#include "stm32f401xc.h"

void UART1_Init_115200_84MHz(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    (void)RCC->APB2ENR;

    /* PA9 = USART1_TX (AF7) */
    GPIOA->MODER &= ~(3U << (9U * 2));
    GPIOA->MODER |=  (2U << (9U * 2));
    GPIOA->AFR[1] &= ~(0xFU << ((9U - 8U) * 4));
    GPIOA->AFR[1] |=  (7U   << ((9U - 8U) * 4));

    /* PA10 = USART1_RX (AF7) */
    GPIOA->MODER &= ~(3U << (10U * 2));
    GPIOA->MODER |=  (2U << (10U * 2));
    GPIOA->AFR[1] &= ~(0xFU << ((10U - 8U) * 4));
    GPIOA->AFR[1] |=  (7U   << ((10U - 8U) * 4));

    USART1->CR1 = 0;
    USART1->BRR = 0x2D93;          /* 115200 @ 84MHz APB2 */
    USART1->CR1 |= USART_CR1_TE
                |  USART_CR1_RE
                |  USART_CR1_UE;
}

void UART1_WriteChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) { }
    USART1->DR = (uint8_t)c;
}

void UART1_WriteString(const char *s)
{
    while (*s) {
        if (*s == '\n') UART1_WriteChar('\r');
        UART1_WriteChar(*s++);
    }
}

void UART1_WriteU32(uint32_t v)
{
    char buf[11];
    int i = 0;

    if (v == 0) { UART1_WriteChar('0'); return; }

    while (v > 0 && i < 10) {
        buf[i++] = (char)('0' + (v % 10U));
        v /= 10U;
    }
    for (int j = i - 1; j >= 0; j--)
        UART1_WriteChar(buf[j]);
}