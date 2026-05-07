#include "config.h"
#include "hal.h"
#include "motor.h"
#include "ultrasonic.h"
#include "debug.h"
#include "esp_wifi.h"

volatile uint32_t enc_right = 0; // PA6
volatile uint32_t enc_left  = 0; // PA7

static void Encoder_Init_PA6_PA7(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    GPIOA->MODER &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOA->PUPDR &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOA->PUPDR |=  ((1U << (6*2)) | (1U << (7*2)));

    SYSCFG->EXTICR[1] &= ~((0xFU << 8) | (0xFU << 12));
    EXTI->IMR  |= (1U << 6) | (1U << 7);
    EXTI->RTSR |= (1U << 6) | (1U << 7);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1U << 6)) {
        EXTI->PR = (1U << 6);
        enc_right++;
    }
    if (EXTI->PR & (1U << 7)) {
        EXTI->PR = (1U << 7);
        enc_left++;
    }
}

/* --- Sensor pins --- */
#define FRONT_TRIG 4
#define FRONT_ECHO 5
#define LEFT_TRIG  6
#define LEFT_ECHO  7
#define RIGHT_TRIG 8
#define RIGHT_ECHO 9

/* --- Thresholds (tune these) --- */
#define FRONT_WALL_CM 15
#define SIDE_WALL_CM  12

/* --- Speeds (0..999) --- */
#define FWD_SPEED     650
#define TURN_SPEED    600

/* --- Timed 90° turn (tune for your robot) --- */
#define TURN_TIME_MS  550

typedef enum {
    S0_IDLE = 0,
    S1_FOLLOW_WALL,
    S2_TURN_LEFT,
    S3_TURN_RIGHT,
    S4_WALL_LOST,
    S5_STOP
} FSM_State_t;

static FSM_State_t state = S0_IDLE;
static uint32_t state_start_ms = 0;
static uint32_t last_send_ms = 0;
static uint32_t last_sensor_ms = 0;
static uint32_t last_wifi_ms = 0;
static uint32_t last_enc_ms = 0;

static void set_state(FSM_State_t s)
{
    state = s;
    state_start_ms = HAL_GetTick();
}

int main(void)
{
    System_Init();
    Debug_Init(115200);
    Motor_Init();
    Ultrasonic_Init();
    Encoder_Init_PA6_PA7();

    DBGLN("--- Wall Follower System Started ---");

    ESP_Wifi_Init(115200);
    if (ESP_Wifi_Connect()) {
        DBGLN("[System] WiFi Connected and Ready");
        last_send_ms = HAL_GetTick();
    } else {
        DBGLN("[System] WiFi Connection Failed - Proceeding in offline mode");
    }

    set_state(S1_FOLLOW_WALL);

    while (1) {
        uint32_t now = HAL_GetTick();

        /* Read sensors */
        uint32_t front = Ultrasonic_Read_cm(FRONT_TRIG, FRONT_ECHO);
        uint32_t left  = Ultrasonic_Read_cm(LEFT_TRIG,  LEFT_ECHO);
        uint32_t right = Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);

        /* Print sensor readings every 5 seconds */
        // if (now - last_sensor_ms >= 5000) {
        //     last_sensor_ms = now;
        //     DBGF("F:%lu L:%lu R:%lu\n", front, left, right);
        // }

        /* Print WiFi status every 2 seconds */
        // if (now - last_wifi_ms >= 2000) {
        //     last_wifi_ms = now;
        //     DBGF("[WiFi] Connected=%d\n", ESP_Wifi_IsConnected() ? 1 : 0);
        // }

        /* Print encoder pulses every 1 second */
        if (now - last_enc_ms >= 1000) {
            last_enc_ms = now;
            DBGF("[ENC] R:%lu  L:%lu\n", enc_right, enc_left);
            enc_right = 0;
            enc_left  = 0;
        }

        /* Send front distance over WiFi every interval */
        if (ESP_Wifi_IsConnected() && (now - last_send_ms >= SEND_INTERVAL_MS)) {
            last_send_ms = now;
            if (!ESP_Wifi_SendData((float)front)) {
                DBGLN("[WiFi] Send failed");
            } else {
                DBGLN("[WiFi] Sent OK");
            }
        }

        switch (state) {
        case S1_FOLLOW_WALL:
            Motor_Drive(&MotorA, MOTOR_FORWARD, FWD_SPEED);
            Motor_Drive(&MotorB, MOTOR_FORWARD, FWD_SPEED);

            if (front < FRONT_WALL_CM) {
                if (left > SIDE_WALL_CM && right <= SIDE_WALL_CM) {
                    set_state(S2_TURN_LEFT);
                    DBGLN("TURN LEFT");
                } else if (right > SIDE_WALL_CM && left <= SIDE_WALL_CM) {
                    set_state(S3_TURN_RIGHT);
                    DBGLN("TURN RIGHT");
                } else if (left > SIDE_WALL_CM && right > SIDE_WALL_CM) {
                    set_state(S2_TURN_LEFT);
                    DBGLN("TURN LEFT (both open)");
                } else {
                    set_state(S4_WALL_LOST);
                    DBGLN("WALL LOST");
                }
            }
            break;

        case S2_TURN_LEFT:
            Motor_Drive(&MotorA, MOTOR_BACKWARD, TURN_SPEED);
            Motor_Drive(&MotorB, MOTOR_FORWARD,  TURN_SPEED);
            if (now - state_start_ms >= TURN_TIME_MS) {
                set_state(S1_FOLLOW_WALL);
            }
            break;

        case S3_TURN_RIGHT:
            Motor_Drive(&MotorA, MOTOR_FORWARD,  TURN_SPEED);
            Motor_Drive(&MotorB, MOTOR_BACKWARD, TURN_SPEED);
            if (now - state_start_ms >= TURN_TIME_MS) {
                set_state(S1_FOLLOW_WALL);
            }
            break;

        case S4_WALL_LOST:
            Motor_Drive(&MotorA, MOTOR_FORWARD, 400);
            Motor_Drive(&MotorB, MOTOR_FORWARD, 400);
            if (front < FRONT_WALL_CM || left < SIDE_WALL_CM || right < SIDE_WALL_CM) {
                set_state(S1_FOLLOW_WALL);
            }
            break;

        case S0_IDLE:
        case S5_STOP:
        default:
            Motor_Stop(&MotorA);
            Motor_Stop(&MotorB);
            break;
        }

        HAL_Delay(100);
    }
}