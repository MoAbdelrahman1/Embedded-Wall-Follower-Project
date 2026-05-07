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

/* WiFi credentials */
// #define WIFI_SSID        "Azima"
// #define WIFI_PASS        "Azima2005"

// /* TCP server (your laptop running receiver.py) */
// #define SERVER_IP        "172.29.64.1"
// #define SERVER_PORT      "5000"
/* --- Robot WiFi Access Point Settings --- */
#define ROBOT_SSID       "STM32_Wall_Follower"
#define ROBOT_PASS       "12345678"

/* The IP your laptop will get when connected to the Robot */
#define SERVER_IP        "192.168.4.2"
#define SERVER_PORT      "5000"

/* UART pins for ESP-01S (USART6) */
#define ESP_TX_PIN       PA11
#define ESP_RX_PIN       PA12

/* Timing */
#define SEND_INTERVAL_MS 2000

#endif