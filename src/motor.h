#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

typedef struct {
    uint8_t en_pin;
    uint8_t in1_pin;
    uint8_t in2_pin;
    uint8_t in3_pin;
    uint8_t in4_pin;
} Motor_t;

typedef enum {
    MOTOR_STOP     = 0,
    MOTOR_FORWARD  = 1,
    MOTOR_BACKWARD = 2
} MotorDirection_t;

extern Motor_t MotorA;
extern Motor_t MotorB;

void Motor_Init(void);
void Motor_SetSpeed(Motor_t *motor, uint16_t speed);
void Motor_SetDirection(Motor_t *motor, MotorDirection_t direction);
void Motor_Drive(Motor_t *motor, MotorDirection_t direction, uint16_t speed);
void Motor_Stop(Motor_t *motor);

#endif