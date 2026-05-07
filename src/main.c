// #include "config.h"
// #include "hal.h"
// #include "motor.h"
// #include "ultrasonic.h"
// #include "debug.h"
// #include "esp_wifi.h"

// volatile uint32_t enc_right = 0; // per-second counter (PA6)
// volatile uint32_t enc_left  = 0; // per-second counter (PA7)

// static uint32_t enc_right_total = 0; // total counts since boot
// static uint32_t enc_left_total  = 0;

// #define TURN_PULSES_90  10  // initial estimate, calibrate if needed

// static void Encoder_Init_PA6_PA7(void)
// {
//     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
//     RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

//     GPIOA->MODER &= ~((3U << (6*2)) | (3U << (7*2)));
//     GPIOA->PUPDR &= ~((3U << (6*2)) | (3U << (7*2)));
//     GPIOA->PUPDR |=  ((1U << (6*2)) | (1U << (7*2)));

//     SYSCFG->EXTICR[1] &= ~((0xFU << 8) | (0xFU << 12));
//     EXTI->IMR  |= (1U << 6) | (1U << 7);
//     EXTI->RTSR |= (1U << 6) | (1U << 7);

//     NVIC_EnableIRQ(EXTI9_5_IRQn);
// }

// void EXTI9_5_IRQHandler(void)
// {
//     if (EXTI->PR & (1U << 6)) {
//         EXTI->PR = (1U << 6);
//         enc_right++;
//         enc_right_total++;
//     }
//     if (EXTI->PR & (1U << 7)) {
//         EXTI->PR = (1U << 7);
//         enc_left++;
//         enc_left_total++;
//     }
// }

// /* --- Sensor pins --- */
// #define FRONT_TRIG 4
// #define FRONT_ECHO 5
// #define LEFT_TRIG  6
// #define LEFT_ECHO  7
// #define RIGHT_TRIG 8
// #define RIGHT_ECHO 9

// /* --- Thresholds (tune these) --- */
// #define FRONT_WALL_CM 15
// #define SIDE_WALL_CM  12

// /* --- Speeds (0..999) --- */
// #define FWD_SPEED     650
// #define TURN_SPEED    600

// typedef enum {
//     S0_IDLE = 0,
//     S1_FOLLOW_WALL,
//     S2_TURN_LEFT,
//     S3_TURN_RIGHT,
//     S4_WALL_LOST,
//     S5_STOP
// } FSM_State_t;

// static FSM_State_t state = S0_IDLE;
// static FSM_State_t prev_state = S0_IDLE;

// static uint32_t state_start_ms = 0;
// static uint32_t last_send_ms = 0;
// static uint32_t last_enc_ms = 0;

// static uint32_t turn_start_r = 0;
// static uint32_t turn_start_l = 0;

// static void set_state(FSM_State_t s)
// {
//     state = s;
//     state_start_ms = HAL_GetTick();
// }

// int main(void)
// {
//     System_Init();
//     Debug_Init(115200);
//     Motor_Init();
//     Ultrasonic_Init();
//     Encoder_Init_PA6_PA7();

//     DBGLN("--- Wall Follower System Started ---");

//     ESP_Wifi_Init(115200);
//     if (ESP_Wifi_Connect()) {
//         DBGLN("[System] WiFi Connected and Ready");
//         last_send_ms = HAL_GetTick();
//     } else {
//         DBGLN("[System] WiFi Connection Failed - Proceeding in offline mode");
//     }

//     set_state(S1_FOLLOW_WALL);

//     while (1) {
//         uint32_t now = HAL_GetTick();

//         /* Read sensors */
//         uint32_t front = Ultrasonic_Read_cm(FRONT_TRIG, FRONT_ECHO);
//         uint32_t left  = Ultrasonic_Read_cm(LEFT_TRIG,  LEFT_ECHO);
//         uint32_t right = Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);

//         /* Print encoder pulses every 1 second */
//         if (now - last_enc_ms >= 1000) {
//             last_enc_ms = now;
//             DBGF("[ENC] R:%lu  L:%lu\n", enc_right, enc_left);
//             enc_right = 0;
//             enc_left  = 0;
//         }

//         /* Send front distance over WiFi every interval */
//         if (ESP_Wifi_IsConnected() && (now - last_send_ms >= SEND_INTERVAL_MS)) {
//             last_send_ms = now;
//             if (!ESP_Wifi_SendData((float)front)) {
//                 DBGLN("[WiFi] Send failed");
//             } else {
//                 DBGLN("[WiFi] Sent OK");
//             }
//         }

//         /* Detect state change to capture turn start counts */
//         if (state != prev_state) {
//             if (state == S2_TURN_LEFT || state == S3_TURN_RIGHT) {
//                 turn_start_r = enc_right_total;
//                 turn_start_l = enc_left_total;
//             }
//             prev_state = state;
//         }

//         switch (state) {
//         case S1_FOLLOW_WALL:
//             Motor_Drive(&MotorA, MOTOR_FORWARD, FWD_SPEED);
//             Motor_Drive(&MotorB, MOTOR_FORWARD, FWD_SPEED);

//             if (front < FRONT_WALL_CM) {
//                 if (left > SIDE_WALL_CM && right <= SIDE_WALL_CM) {
//                     set_state(S2_TURN_LEFT);
//                     DBGLN("TURN LEFT");
//                 } else if (right > SIDE_WALL_CM && left <= SIDE_WALL_CM) {
//                     set_state(S3_TURN_RIGHT);
//                     DBGLN("TURN RIGHT");
//                 } else if (left > SIDE_WALL_CM && right > SIDE_WALL_CM) {
//                     set_state(S2_TURN_LEFT); /* prefer left */
//                     DBGLN("TURN LEFT (both open)");
//                 } else {
//                     set_state(S4_WALL_LOST);
//                     DBGLN("WALL LOST");
//                 }
//             }
//             break;

//         case S2_TURN_LEFT:
//             Motor_Drive(&MotorA, MOTOR_BACKWARD, TURN_SPEED);
//             Motor_Drive(&MotorB, MOTOR_FORWARD,  TURN_SPEED);

//             if ((enc_right_total - turn_start_r) >= TURN_PULSES_90 ||
//                 (enc_left_total  - turn_start_l) >= TURN_PULSES_90) {
//                 set_state(S1_FOLLOW_WALL);
//             }
//             break;

//         case S3_TURN_RIGHT:
//             Motor_Drive(&MotorA, MOTOR_FORWARD,  TURN_SPEED);
//             Motor_Drive(&MotorB, MOTOR_BACKWARD, TURN_SPEED);

//             if ((enc_right_total - turn_start_r) >= TURN_PULSES_90 ||
//                 (enc_left_total  - turn_start_l) >= TURN_PULSES_90) {
//                 set_state(S1_FOLLOW_WALL);
//             }
//             break;

//         case S4_WALL_LOST:
//             Motor_Drive(&MotorA, MOTOR_FORWARD, 400);
//             Motor_Drive(&MotorB, MOTOR_FORWARD, 400);
//             if (front < FRONT_WALL_CM || left < SIDE_WALL_CM || right < SIDE_WALL_CM) {
//                 set_state(S1_FOLLOW_WALL);
//             }
//             break;

//         case S0_IDLE:
//         case S5_STOP:
//         default:
//             Motor_Stop(&MotorA);
//             Motor_Stop(&MotorB);
//             break;
//         }

//         HAL_Delay(100);
//     }
// }
#include "config.h"
#include "hal.h"
#include "motor.h"
#include "debug.h"

/* Encoder pins: Right = PA6, Left = PA7 */
volatile uint32_t enc_right = 0;
volatile uint32_t enc_left  = 0;

static uint32_t enc_right_total = 0;
static uint32_t enc_left_total  = 0;

#define TURN_SPEED      450
#define TURN_PULSES_90  200   // start with 10, calibrate

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
        enc_right_total++;
        enc_right_total++;
    }
    if (EXTI->PR & (1U << 7)) {
        EXTI->PR = (1U << 7);
        enc_left++;
        enc_left_total++;
    }
}

static void record_turn(char dir)
{
    if (turn_count < MAX_TURNS) {
        turn_seq[turn_count] = dir;
        turn_count++;
        turn_seq[turn_count] = '\0';
    }
}

int main(void)
{
    System_Init();
    Debug_Init(115200);
    Motor_Init();
    Encoder_Init_PA6_PA7();

    DBGLN("=== 90 DEG TURN TEST ===");

    uint32_t start_r = enc_right_total;
    uint32_t start_l = enc_left_total;

    /* Start turning left in place */
     HAL_Delay(5000);
    Motor_Drive(&MotorA, MOTOR_FORWARD, TURN_SPEED);
    Motor_Drive(&MotorB, MOTOR_FORWARD,  TURN_SPEED);

    while (1) {
        uint32_t dr = enc_right_total - start_r;
        uint32_t dl = enc_left_total  - start_l;

        DBGF("[TURN] R:%lu L:%lu\n", dr, dl);

        if (dr >= TURN_PULSES_90 || dl >= TURN_PULSES_90) {
            Motor_Stop(&MotorA);
            Motor_Stop(&MotorB);
            DBGLN("=== TURN DONE ===");
            while (1) { HAL_Delay(1000); }
        }

        HAL_Delay(200);
    }
}
