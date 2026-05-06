#include "motor.h"
#include "config.h"
#include "stm32f401xc.h"

Motor_t MotorA = { MOTOR_A_EN_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, 0, 0 };
Motor_t MotorB = { MOTOR_B_EN_PIN, 0, 0, MOTOR_B_IN3_PIN, MOTOR_B_IN4_PIN };

void Motor_Init(void)
{
    /* Enable clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM5EN;
    (void)RCC->APB1ENR; /* dummy read - let clock settle */

    /* PA1, PA2, PA3, PA4 → GPIO output (direction pins) */
    GPIOA->MODER &= ~((3U<<2)|(3U<<4)|(3U<<6)|(3U<<8));
    GPIOA->MODER |=  ((1U<<2)|(1U<<4)|(1U<<6)|(1U<<8));
    GPIOA->OTYPER &= ~((1U<<1)|(1U<<2)|(1U<<3)|(1U<<4)); /* push-pull */

    /* PA0 → AF2 = TIM5_CH1 (Motor A PWM) */
    GPIOA->MODER  &= ~(3U << 0);
    GPIOA->MODER  |=  (2U << 0);
    GPIOA->AFR[0] &= ~(0xFU << 0);
    GPIOA->AFR[0] |=  (2U << 0);   /* AF2 for TIM5 */

    /* PA5 → AF1 = TIM2_CH1 (Motor B PWM) */
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (2U << 10);
    GPIOA->AFR[0] &= ~(0xFU << 20);
    GPIOA->AFR[0] |=  (1U << 20);  /* AF1 for TIM2 */

    /* TIM5 → Motor A PWM */
    TIM5->PSC   = 83U;
    TIM5->ARR   = PWM_MAX;
    TIM5->CCR1  = 0U;
    TIM5->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM5->CCER  = TIM_CCER_CC1E;
    TIM5->EGR   = TIM_EGR_UG;  /* load shadow registers NOW */
    TIM5->CR1   = TIM_CR1_ARPE | TIM_CR1_CEN;

    /* TIM2 → Motor B PWM */
    TIM2->PSC   = 83U;
    TIM2->ARR   = PWM_MAX;
    TIM2->CCR1  = 0U;
    TIM2->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM2->CCER  = TIM_CCER_CC1E;
    TIM2->EGR   = TIM_EGR_UG;
    TIM2->CR1   = TIM_CR1_ARPE | TIM_CR1_CEN;
}

void Motor_SetSpeed(Motor_t *motor, uint16_t speed)
{
    if (speed > PWM_MAX) speed = PWM_MAX;

    if (motor->en_pin == MOTOR_A_EN_PIN) {
        TIM5->CCR1 = speed;
    } else {
        TIM2->CCR1 = speed;
    }
}

void Motor_SetDirection(Motor_t *motor, MotorDirection_t direction)
{
    if (motor->en_pin == MOTOR_A_EN_PIN) {
        if (direction == MOTOR_FORWARD) {
            GPIOA->BSRR = (1U << 1);         /* PA1 high */
            GPIOA->BSRR = (1U << (2+16));    /* PA2 low  */
        } else if (direction == MOTOR_BACKWARD) {
            GPIOA->BSRR = (1U << (1+16));    /* PA1 low  */
            GPIOA->BSRR = (1U << 2);         /* PA2 high */
        } else {
            GPIOA->BSRR = (1U << (1+16));
            GPIOA->BSRR = (1U << (2+16));
        }
    } else {
        if (direction == MOTOR_FORWARD) {
            GPIOA->BSRR = (1U << 3);
            GPIOA->BSRR = (1U << (4+16));
        } else if (direction == MOTOR_BACKWARD) {
            GPIOA->BSRR = (1U << (3+16));
            GPIOA->BSRR = (1U << 4);
        } else {
            GPIOA->BSRR = (1U << (3+16));
            GPIOA->BSRR = (1U << (4+16));
        }
    }
}

void Motor_Drive(Motor_t *motor, MotorDirection_t direction, uint16_t speed)
{
    Motor_SetDirection(motor, direction);
    Motor_SetSpeed(motor, speed);
}

void Motor_Stop(Motor_t *motor)
{
    Motor_SetDirection(motor, MOTOR_STOP);
    Motor_SetSpeed(motor, 0);
}