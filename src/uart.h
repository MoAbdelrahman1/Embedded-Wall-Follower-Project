#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART1_Init_115200_84MHz(void);
void UART1_WriteChar(char c);
void UART1_WriteString(const char *s);
void UART1_WriteU32(uint32_t v);      /* ← still needed by main.c */

#endif /* UART_H */