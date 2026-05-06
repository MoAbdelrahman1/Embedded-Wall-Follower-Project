#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

void Ultrasonic_Init(void);
uint32_t Ultrasonic_Read_cm(void);

#endif