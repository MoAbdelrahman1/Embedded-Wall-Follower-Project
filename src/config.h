#ifndef CONFIG_H
#define CONFIG_H

/* Motor A pins (PA0=EN, PA1=IN1, PA2=IN2) */
#define MOTOR_A_EN_PIN    0U
#define MOTOR_A_IN1_PIN   1U
#define MOTOR_A_IN2_PIN   2U

/* Motor B pins (PA5=EN, PA3=IN3, PA4=IN4) */
#define MOTOR_B_EN_PIN    5U
#define MOTOR_B_IN3_PIN   3U
#define MOTOR_B_IN4_PIN   4U

/* PWM: PSC=83, ARR=999 → 1 kHz PWM (84MHz / 84 / 1000) */
#define PWM_MAX           999U
#define BASE_SPEED        700U   /* ~70% duty cycle */
/* Ultrasonic sensor (HC-SR04) */
#define TRIG_PIN        0U   /* PB0 */
#define ECHO_PIN        1U   /* PB1 */
#define STOP_DISTANCE   15U  /* stop if wall closer than 15 cm */

#endif