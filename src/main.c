/* ============================================================
 * main.c – Refined, Sensor-Based Turning (side-specific stop)
 * ============================================================ */

#include "config.h"
#include "hal.h"
#include "motor.h"
#include "ultrasonic.h"
#include "debug.h"
#include "esp_wifi.h"
#include <string.h>
#include <stdio.h>

/* ── Sensor pins ─────────────────────────────────────────── */
#define FRONT_TRIG  4
#define FRONT_ECHO  5
#define LEFT_TRIG   6
#define LEFT_ECHO   7
#define RIGHT_TRIG  8
#define RIGHT_ECHO  9

/* ── Distance thresholds ────────────────────────────────── */
#define FRONT_SLOW_CM        35
#define FRONT_STOP_CM        26
#define FRONT_CLEAR_CM       30

#define SIDE_OPEN_CM         35
#define TARGET_SIDE_CM       12
#define SIDE_TOLERANCE_CM     3

#define SIDE_NEAR_CM          8
#define SIDE_ENTER_CM        25   // used for side-specific turn finish

/* ── Speeds ─────────────────────────────────────────────── */
#define FWD_SPEED            600
#define SLOW_SPEED           360
#define PRE_TURN_SPEED       260

#define CORRECTION_SPEED      55
#define CLOSE_CORRECTION     120

#define TURN_SPEED           320

/* ── Safety / timing ────────────────────────────────────── */
#define TURN_TIMEOUT_MS      900
#define POST_TURN_MS         250

#define MAX_TURNS 32

typedef enum {
    S_IDLE = 0,
    S_DRIVE,
    S_PRE_TURN,
    S_TURN_LEFT,
    S_TURN_RIGHT,
    S_POST_TURN,
    S_STOP
} FSM_State_t;

/* ── Globals ────────────────────────────────────────────── */
static FSM_State_t state = S_IDLE;

static uint8_t turn_count = 0;
static char turn_seq[MAX_TURNS];

static uint32_t turn_timer = 0;

/* ── Helpers ────────────────────────────────────────────── */

static uint16_t clamp(int32_t v)
{
    if (v < 0) return 0;
    if (v > 999) return 999;
    return (uint16_t)v;
}

static void drive(uint16_t l, uint16_t r)
{
    Motor_Drive(&MotorA, MOTOR_FORWARD, l);
    Motor_Drive(&MotorB, MOTOR_FORWARD, r);
}

static void stop_robot(void)
{
    Motor_Stop(&MotorA);
    Motor_Stop(&MotorB);
}

static void spin_left(void)
{
    Motor_Drive(&MotorA, MOTOR_BACKWARD, TURN_SPEED);
    Motor_Drive(&MotorB, MOTOR_FORWARD,  TURN_SPEED);
}

static void spin_right(void)
{
    Motor_Drive(&MotorA, MOTOR_FORWARD,  TURN_SPEED);
    Motor_Drive(&MotorB, MOTOR_BACKWARD, TURN_SPEED);
}

/* active brake to reduce overshoot */
static void brake_after_left(void)
{
    Motor_Drive(&MotorA, MOTOR_FORWARD, 120);
    Motor_Drive(&MotorB, MOTOR_BACKWARD, 120);
    HAL_Delay(15);
    stop_robot();
}

static void brake_after_right(void)
{
    Motor_Drive(&MotorA, MOTOR_BACKWARD, 120);
    Motor_Drive(&MotorB, MOTOR_FORWARD, 120);
    HAL_Delay(15);
    stop_robot();
}

static void log_turn(char dir)
{
    if (turn_count < MAX_TURNS)
        turn_seq[turn_count] = dir;

    turn_count++;

    DBGF("[TURN #%d] %c\n", turn_count, dir);

    if (ESP_Wifi_IsConnected()) {
        char msg[32];
        snprintf(msg, sizeof(msg),
                 "TURN:%c:%d\n",
                 dir,
                 turn_count);

        ESP_Wifi_SendText(msg);
    }
}

static void send_summary(void)
{
    char buf[128];
    int pos = 0;

    pos += snprintf(buf + pos,
                    sizeof(buf) - pos,
                    "Turns: %d\nSequence: ",
                    turn_count);

    for (uint8_t i = 0;
         i < turn_count && i < MAX_TURNS;
         i++) {

        if (i > 0)
            pos += snprintf(buf + pos,
                            sizeof(buf) - pos,
                            ", ");

        pos += snprintf(buf + pos,
                        sizeof(buf) - pos,
                        "%c",
                        turn_seq[i]);
    }

    pos += snprintf(buf + pos,
                    sizeof(buf) - pos,
                    "\n");

    DBGF("%s", buf);

    if (ESP_Wifi_IsConnected())
        ESP_Wifi_SendText(buf);
}

/* ── MAIN ───────────────────────────────────────────────── */

int main(void)
{
    System_Init();

    Debug_Init(115200);

    Motor_Init();
    Ultrasonic_Init();

    ESP_Wifi_Init(115200);

    if (ESP_Wifi_Connect())
        DBGLN("[WiFi] Connected");
    else
        DBGLN("[WiFi] Offline mode");

    DBGLN("=== Refined FSM Start ===");

    state = S_DRIVE;

    while (1) {

        uint32_t front =
            Ultrasonic_Read_cm(FRONT_TRIG, FRONT_ECHO);

        uint32_t left =
            Ultrasonic_Read_cm(LEFT_TRIG, LEFT_ECHO);

        uint32_t right =
            Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);

        switch (state) {

        /* ───────── DRIVE ───────── */

        case S_DRIVE: {

            if (front < FRONT_STOP_CM) {
                state = S_PRE_TURN;
                break;
            }

            uint16_t base =
                (front < FRONT_SLOW_CM)
                ? PRE_TURN_SPEED
                : FWD_SPEED;

            int32_t lspd = base;
            int32_t rspd = base;

            /* wall too close */
            if (left <= SIDE_NEAR_CM && left < right) {
                lspd = SLOW_SPEED;
                rspd = SLOW_SPEED - CLOSE_CORRECTION;
            }
            else if (right <= SIDE_NEAR_CM && right < left) {
                lspd = SLOW_SPEED - CLOSE_CORRECTION;
                rspd = SLOW_SPEED;
            }
            else {
                /* center using left-right error */
                int32_t err = (int32_t)left - (int32_t)right;

                if (err > SIDE_TOLERANCE_CM)
                    lspd -= CORRECTION_SPEED;
                else if (err < -SIDE_TOLERANCE_CM)
                    rspd -= CORRECTION_SPEED;
            }

            drive(clamp(lspd), clamp(rspd));
            break;
        }

        /* ───────── PRE TURN ───────── */

        case S_PRE_TURN: {

            stop_robot();
            HAL_Delay(40);

            left  = Ultrasonic_Read_cm(LEFT_TRIG, LEFT_ECHO);
            right = Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);

            bool left_open  = (left  >= SIDE_OPEN_CM);
            bool right_open = (right >= SIDE_OPEN_CM);

            if (left_open && !right_open) {
                log_turn('L');
                turn_timer = HAL_GetTick();
                state = S_TURN_LEFT;
            }
            else if (right_open && !left_open) {
                log_turn('R');
                turn_timer = HAL_GetTick();
                state = S_TURN_RIGHT;
            }
            else if (left_open && right_open) {
                log_turn('L');
                turn_timer = HAL_GetTick();
                state = S_TURN_LEFT;
            }
            else {
                /* dead-end U-turn */
                log_turn('L');
                log_turn('L');
                turn_timer = HAL_GetTick();
                state = S_TURN_LEFT;
            }

            break;
        }

        /* ───────── TURN LEFT ───────── */

        case S_TURN_LEFT:

            spin_left();

            // stop when left wall appears (entered new lane) and front is clear
            if (front > FRONT_CLEAR_CM &&
                left < SIDE_ENTER_CM) {

                brake_after_left();
                turn_timer = HAL_GetTick();
                state = S_POST_TURN;
            }
            else if (HAL_GetTick() - turn_timer > TURN_TIMEOUT_MS) {
                brake_after_left();
                turn_timer = HAL_GetTick();
                state = S_POST_TURN;
            }

            break;

        /* ───────── TURN RIGHT ───────── */

        case S_TURN_RIGHT:

            spin_right();

            // stop when right wall appears (entered new lane) and front is clear
            if (front > FRONT_CLEAR_CM &&
                right < SIDE_ENTER_CM) {

                brake_after_right();
                turn_timer = HAL_GetTick();
                state = S_POST_TURN;
            }
            else if (HAL_GetTick() - turn_timer > TURN_TIMEOUT_MS) {
                brake_after_right();
                turn_timer = HAL_GetTick();
                state = S_POST_TURN;
            }

            break;

        /* ───────── POST TURN ───────── */

        case S_POST_TURN:

            // short straight recovery
            drive(SLOW_SPEED, SLOW_SPEED);

            if (HAL_GetTick() - turn_timer >= POST_TURN_MS) {
                state = S_DRIVE;
            }
            break;

        /* ───────── STOP ───────── */

        case S_STOP:

            stop_robot();
            send_summary();
            while (1);

        default:
            break;
        }

        HAL_Delay(5);
    }
}