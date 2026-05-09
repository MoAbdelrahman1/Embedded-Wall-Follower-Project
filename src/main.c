/* ============================================================
 * main.c  –  90° Turn Detection Test
 * Drop this in place of your existing main.c
 * Tune TURN_DURATION_MS on the bench before committing.
 * ============================================================ */

#include "config.h"
#include "hal.h"
#include "motor.h"
#include "ultrasonic.h"
#include "debug.h"
#include "esp_wifi.h"
#include <string.h>
#include <stdio.h>

/* ── Sensor pins (GPIOB) ─────────────────────────────────── */
#define FRONT_TRIG  4
#define FRONT_ECHO  5
#define LEFT_TRIG   6
#define LEFT_ECHO   7
#define RIGHT_TRIG  8
#define RIGHT_ECHO  9

/* ── Distance thresholds (cm) ────────────────────────────── */
#define FRONT_SLOW_CM        35
#define FRONT_STOP_CM        27  /* wall is close - evaluate turn */
#define FRONT_RESUME_CM      30
#define SIDE_OPEN_CM         35   /* side reads THIS or more → corridor is open */
#define TARGET_SIDE_CM       12
#define SIDE_TOLERANCE_CM     4
#define MIN_SIDE_CLEAR_CM     6
#define SIDE_AVOID_CM         9

/* ── Motor speeds (0–999) ────────────────────────────────── */
#define FWD_SPEED            620
#define SLOW_SPEED           380
#define PRE_TURN_SPEED       260
#define CORRECTION_SPEED     140
#define CLOSE_CORRECTION     200
#define TURN_SPEED           450   /* each motor during spin */

/* ── Turn timing — TUNE THIS ON YOUR ROBOT ───────────────── */
/* Spin one motor fwd, other bwd at TURN_SPEED.
 * Start at 500 ms, bump ±50 ms until you get clean 90°.      */
#define TURN_PULSES_90       240
#define TURN_TIMEOUT_MS      1500
#define TURN_DURATION_MS     450
#define POST_TURN_NO_ADJUST_MS 400
#define POST_TURN_SLOW_ADJUST_MS 600

/* ── Max turns the track can have ───────────────────────────*/
#define MAX_TURNS            32

/* ── FSM states ──────────────────────────────────────────── */
typedef enum {
    S_IDLE = 0,
    S_DRIVE,        /* normal wall-following */
    S_PRE_TURN,     /* slowing down before turn */
    S_TURN_LEFT,
    S_TURN_RIGHT,
    S_POST_TURN,    /* short straight to re-acquire walls */
    S_STOP
} FSM_State_t;

/* ── Global turn log ─────────────────────────────────────── */
static FSM_State_t  state       = S_IDLE;
static uint8_t      turn_count  = 0;
static char         turn_seq[MAX_TURNS]; /* 'L' or 'R' */
static uint32_t     turn_timer  = 0;     /* ms timestamp */
static uint32_t     slow_adjust_until_ms = 0;
static volatile uint32_t enc_right_total = 0;
static volatile uint32_t enc_left_total  = 0;
static uint32_t turn_start_right = 0;
static uint32_t turn_start_left  = 0;
static uint32_t turn_target_pulses = TURN_PULSES_90;

static void Encoder_Init_PA6_PA7(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    GPIOA->MODER &= ~((3U << (6U * 2U)) | (3U << (7U * 2U)));
    GPIOA->PUPDR &= ~((3U << (6U * 2U)) | (3U << (7U * 2U)));
    GPIOA->PUPDR |=  ((1U << (6U * 2U)) | (1U << (7U * 2U)));

    SYSCFG->EXTICR[1] &= ~((0xFU << 8U) | (0xFU << 12U));
    EXTI->IMR  |= (1U << 6U) | (1U << 7U);
    EXTI->RTSR |= (1U << 6U) | (1U << 7U);
    EXTI->FTSR &= ~((1U << 6U) | (1U << 7U));
    EXTI->PR = (1U << 6U) | (1U << 7U);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1U << 6U)) {
        EXTI->PR = (1U << 6U);
        enc_right_total++;
    }

    if (EXTI->PR & (1U << 7U)) {
        EXTI->PR = (1U << 7U);
        enc_left_total++;
    }
}

/* ── Helpers ─────────────────────────────────────────────── */
static uint16_t clamp(int32_t v)
{
    if (v < 0)   return 0;
    if (v > 999) return 999;
    return (uint16_t)v;
}

static void drive(uint16_t l, uint16_t r)
{
    Motor_Drive(&MotorA, MOTOR_FORWARD,  l);
    Motor_Drive(&MotorB, MOTOR_FORWARD,  r);
}

static void stop_robot(void)
{
    Motor_Stop(&MotorA);
    Motor_Stop(&MotorB);
}

/* Spin left: left motor back, right motor fwd */
static void spin_left(void)
{
    Motor_Drive(&MotorA, MOTOR_BACKWARD, TURN_SPEED);
    Motor_Drive(&MotorB, MOTOR_FORWARD,  TURN_SPEED);
}

/* Spin right: left motor fwd, right motor back */
static void spin_right(void)
{
    Motor_Drive(&MotorA, MOTOR_FORWARD,  TURN_SPEED);
    Motor_Drive(&MotorB, MOTOR_BACKWARD, TURN_SPEED);
}

/* Record turn and send over WiFi */
static void log_turn(char dir)
{
    if (turn_count < MAX_TURNS)
        turn_seq[turn_count] = dir;
    turn_count++;

    /* Debug immediately */
    DBGF("[TURN #%d] %c\n", turn_count, dir);

    /* Send live over WiFi if connected */
    if (ESP_Wifi_IsConnected()) {
        char msg[32];
        snprintf(msg, sizeof(msg), "TURN:%c:%d\n", dir, turn_count);
        ESP_Wifi_SendText(msg);
    }
}

static void start_turn(FSM_State_t turn_state, char dir, uint32_t target_pulses)
{
    log_turn(dir);
    turn_start_right = enc_right_total;
    turn_start_left = enc_left_total;
    turn_target_pulses = target_pulses;
    turn_timer = HAL_GetTick();
    state = turn_state;
}

/* Build and send the final summary string */
static void send_summary(void)
{
    char buf[128];
    int  pos = 0;

    pos += snprintf(buf + pos, sizeof(buf) - pos,
                    "Turns: %d\nSequence: ", turn_count);

    for (uint8_t i = 0; i < turn_count && i < MAX_TURNS; i++) {
        if (i > 0)
            pos += snprintf(buf + pos, sizeof(buf) - pos, ", ");
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%c", turn_seq[i]);
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");

    DBGF("%s", buf);
    if (ESP_Wifi_IsConnected())
        ESP_Wifi_SendText(buf);
}

/* ── Main ─────────────────────────────────────────────────── */
int main(void)
{
    System_Init();
    Debug_Init(115200);
    Motor_Init();
    Ultrasonic_Init();
    Encoder_Init_PA6_PA7();

    ESP_Wifi_Init(115200);
    if (ESP_Wifi_Connect())
        DBGLN("[WiFi] Connected");
    else
        DBGLN("[WiFi] Offline mode");

    DBGLN("=== Turn-Test FSM Start ===");
    state = S_DRIVE;

    while (1) {
        uint32_t front = Ultrasonic_Read_cm(FRONT_TRIG, FRONT_ECHO);
        uint32_t left  = Ultrasonic_Read_cm(LEFT_TRIG,  LEFT_ECHO);
        uint32_t right = Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);

        /* ── Live debug every loop (remove after tuning) ── */
        DBGF("F:%3lu L:%3lu R:%3lu  state:%d\n",
             front, left, right, (int)state);

        switch (state) {

        /* ─── DRIVE: normal wall centering ─────────────── */
        case S_DRIVE: {
            /* Approaching something in front → slow and evaluate */
            if (front < FRONT_STOP_CM) {
                state = S_PRE_TURN;
                break;
            }

            uint16_t base = ((front < FRONT_SLOW_CM) ||
                             ((int32_t)(slow_adjust_until_ms - HAL_GetTick()) > 0))
                          ? PRE_TURN_SPEED
                          : FWD_SPEED;
            int32_t  lspd = base, rspd = base;

            if (left <= MIN_SIDE_CLEAR_CM && left < right) {
                lspd = SLOW_SPEED;
                rspd = SLOW_SPEED - CLOSE_CORRECTION;
            } else if (right <= MIN_SIDE_CLEAR_CM && right < left) {
                lspd = SLOW_SPEED - CLOSE_CORRECTION;
                rspd = SLOW_SPEED;
            } else if (left < SIDE_AVOID_CM) {
                rspd -= CORRECTION_SPEED;
            } else if (right < SIDE_AVOID_CM) {
                lspd -= CORRECTION_SPEED;
            } else {
                int32_t err = (int32_t)left - (int32_t)right;
                if      (err >  SIDE_TOLERANCE_CM) lspd -= CORRECTION_SPEED / 2;
                else if (err < -SIDE_TOLERANCE_CM) rspd -= CORRECTION_SPEED / 2;
            }
            drive(clamp(lspd), clamp(rspd));
            break;
        }

        /* ─── PRE_TURN: stop, decide direction ─────────── */
        case S_PRE_TURN: {
            /* If front cleared again (false positive), go back */
            if (front >= FRONT_RESUME_CM) {
                state = S_DRIVE;
                break;
            }

            /* Wall is genuinely blocking → stop and check sides */
            if (front < FRONT_STOP_CM) {
                stop_robot();
                HAL_Delay(150); /* settle before reading sides */

                /* Re-read after settling */
                left  = Ultrasonic_Read_cm(LEFT_TRIG,  LEFT_ECHO);
                right = Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);
                DBGF("[PRE_TURN] F:%lu L:%lu R:%lu\n", front, left, right);

                bool left_open  = (left  >= SIDE_OPEN_CM);
                bool right_open = (right >= SIDE_OPEN_CM);

                if (left_open && !right_open) {
                    start_turn(S_TURN_LEFT, 'L', TURN_PULSES_90);
                } else if (right_open && !left_open) {
                    start_turn(S_TURN_RIGHT, 'R', TURN_PULSES_90);
                } else if (left_open && right_open) {
                    /* T-junction: prefer left (or change to 'R' if needed) */
                    DBGLN("[TURN] T-junction → choosing LEFT");
                    start_turn(S_TURN_LEFT, 'L', TURN_PULSES_90);
                } else {
                    /* Dead end: U-turn (two left spins) — extend duration */
                    DBGLN("[TURN] Dead-end → U-turn");
                    log_turn('L');
                    log_turn('L');
                    turn_start_right = enc_right_total;
                    turn_start_left = enc_left_total;
                    turn_target_pulses = TURN_PULSES_90 * 2U;
                    turn_timer = HAL_GetTick();
                    state = S_TURN_LEFT;
                }
            } else {
                /* Slowing toward front wall */
                drive(PRE_TURN_SPEED, PRE_TURN_SPEED);
            }
            break;
        }

        /* ─── TURN_LEFT: spin until timer expires ───────── */
        case S_TURN_LEFT:
            spin_left();
            if (HAL_GetTick() - turn_timer >= TURN_DURATION_MS) {
                stop_robot();
                HAL_Delay(100);
                turn_timer = HAL_GetTick();
                state = S_POST_TURN;
            }
            break;

        /* ─── TURN_RIGHT: spin until timer expires ──────── */
        case S_TURN_RIGHT:
            spin_right();
            if (HAL_GetTick() - turn_timer >= TURN_DURATION_MS) {
                stop_robot();
                HAL_Delay(100);
                turn_timer = HAL_GetTick();
                state = S_POST_TURN;
            }
            break;

        /* ─── POST_TURN: short straight to re-acquire walls  */
        case S_POST_TURN:
            drive(SLOW_SPEED, SLOW_SPEED);
            /* Give sensors 400 ms to stabilise, then resume */
            if (HAL_GetTick() - turn_timer >= POST_TURN_NO_ADJUST_MS) {
                slow_adjust_until_ms = HAL_GetTick() + POST_TURN_SLOW_ADJUST_MS;
                state = S_DRIVE;
            }
            break;

        /* ─── STOP: end of track (trigger manually for now) */
        case S_STOP:
            stop_robot();
            send_summary();
            while (1); /* hang until reset */
            break;

        case S_IDLE:
        default:
            stop_robot();
            break;
        }

        HAL_Delay(50); /* 20 Hz loop — fast enough, not too spammy */
    }
}
