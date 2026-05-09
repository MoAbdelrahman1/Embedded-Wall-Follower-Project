#include "config.h"
#include "hal.h"
#include "motor.h"
#include "ultrasonic.h"
#include "debug.h"
#include "esp_wifi.h"

/* --- Sensor pins on GPIOB --- */
#define FRONT_TRIG 4
#define FRONT_ECHO 5
#define LEFT_TRIG  6
#define LEFT_ECHO  7
#define RIGHT_TRIG 8
#define RIGHT_ECHO 9

/* --- Track geometry / safety thresholds ---
 * Track width ~= 45 cm, robot width ~= 20 cm, so a centered robot has
 * about 12.5 cm clearance on each side.
 */
#define TARGET_SIDE_CM       12
#define SIDE_TOLERANCE_CM     6
#define MIN_SIDE_CLEAR_CM      6
#define SIDE_AVOID_CM          9
#define FRONT_SLOW_CM         55
#define FRONT_STOP_CM         45
#define FRONT_RESUME_CM       55
#define FRONT_EMERGENCY_CM     9
#define TURN_OPEN_CM          22

/* --- Speeds (0..999) --- */
#define FWD_SPEED            620
#define SLOW_SPEED           420
#define CORRECTION_SPEED     110
#define CLOSE_CORRECTION_SPEED 160
#define TURN_SPEED           430
#define RECOVERY_SPEED       360

/* --- Timed movement (tune for your robot) --- */
#define TURN_TIME_MS         650
#define RECOVERY_TIME_MS     350

typedef enum {
    S0_IDLE = 0,
    S1_DRIVE_CENTERED,
    S2_TURN_LEFT,
    S3_TURN_RIGHT,
    S4_RECOVER_REVERSE,
    S5_STOP
} FSM_State_t;

static FSM_State_t state = S0_IDLE;
static uint32_t state_start_ms = 0;
static FSM_State_t next_state = S1_DRIVE_CENTERED;
static uint8_t front_stop_latched = 0;

static void set_state(FSM_State_t s)
{
    state = s;
    state_start_ms = HAL_GetTick();
}

static uint16_t clamp_speed(int32_t speed)
{
    if (speed < 0) return 0;
    if (speed > 999) return 999;
    return (uint16_t)speed;
}

static void drive_forward(uint16_t left_speed, uint16_t right_speed)
{
    Motor_Drive(&MotorA, MOTOR_FORWARD, left_speed);
    Motor_Drive(&MotorB, MOTOR_FORWARD, right_speed);
}

static void stop_robot(void)
{
    Motor_Stop(&MotorA);
    Motor_Stop(&MotorB);
}

static void recover_then(FSM_State_t after_recovery)
{
    next_state = after_recovery;
    set_state(S4_RECOVER_REVERSE);
}

int main(void)
{
    System_Init();
    Debug_Init(115200);
    Motor_Init();
    Ultrasonic_Init();

    ESP_Wifi_Init(115200);
    if (ESP_Wifi_Connect()) {
        DBGLN("[System] WiFi Connected and Ready");
    } else {
        DBGLN("[System] WiFi Connection Failed - Proceeding in offline mode");
    }

    DBGLN("FSM Start");

    /* Start immediately (no start button). */
    set_state(S1_DRIVE_CENTERED);

    while (1) {
        uint32_t now = HAL_GetTick();

        uint32_t front = Ultrasonic_Read_cm(FRONT_TRIG, FRONT_ECHO);
        uint32_t left  = Ultrasonic_Read_cm(LEFT_TRIG,  LEFT_ECHO);
        uint32_t right = Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);

        // DBGF("F:%lu L:%lu R:%lu\n", front, left, right);

        switch (state) {
        case S1_DRIVE_CENTERED: {
            uint16_t base_speed = (front < FRONT_SLOW_CM) ? SLOW_SPEED : FWD_SPEED;
            int32_t left_speed = base_speed;
            int32_t right_speed = base_speed;
            int32_t side_error;

            if (front < FRONT_STOP_CM) {
                front_stop_latched = 1;
            } else if (front_stop_latched && front >= FRONT_RESUME_CM) {
                front_stop_latched = 0;
            }

            if (front_stop_latched) {
                DBGLN("STOP: front obstacle");
                stop_robot();
                break;
            }

            if (left <= MIN_SIDE_CLEAR_CM && left < right) {
                /* Very close to left wall: slow right motor to drift right. */
                left_speed = SLOW_SPEED;
                right_speed = SLOW_SPEED - CLOSE_CORRECTION_SPEED;
            } else if (right <= MIN_SIDE_CLEAR_CM && right < left) {
                /* Very close to right wall: slow left motor to drift left. */
                left_speed = SLOW_SPEED - CLOSE_CORRECTION_SPEED;
                right_speed = SLOW_SPEED;
            } else if (left < SIDE_AVOID_CM) {
                /* Approaching left wall: slow right motor to drift right. */
                right_speed -= CORRECTION_SPEED;
            } else if (right < SIDE_AVOID_CM) {
                /* Approaching right wall: slow left motor to drift left. */
                left_speed -= CORRECTION_SPEED;
            } else {
                /* Keep centered when both side sensors see valid walls. */
                side_error = (int32_t)left - (int32_t)right;
                if (side_error > SIDE_TOLERANCE_CM) {
                    left_speed -= CORRECTION_SPEED / 2;
                } else if (side_error < -SIDE_TOLERANCE_CM) {
                    right_speed -= CORRECTION_SPEED / 2;
                }
            }

            drive_forward(clamp_speed(left_speed), clamp_speed(right_speed));
            break;
        }

        case S2_TURN_LEFT:
            Motor_Drive(&MotorA, MOTOR_BACKWARD, TURN_SPEED);
            Motor_Drive(&MotorB, MOTOR_FORWARD,  TURN_SPEED);
            if (front <= FRONT_EMERGENCY_CM) {
                DBGLN("RECOVER: unsafe left turn");
                recover_then(S2_TURN_LEFT);
            } else if (now - state_start_ms >= TURN_TIME_MS) {
                set_state(S1_DRIVE_CENTERED);
            }
            break;

        case S3_TURN_RIGHT:
            Motor_Drive(&MotorA, MOTOR_FORWARD,  TURN_SPEED);
            Motor_Drive(&MotorB, MOTOR_BACKWARD, TURN_SPEED);
            if (front <= FRONT_EMERGENCY_CM) {
                DBGLN("RECOVER: unsafe right turn");
                recover_then(S3_TURN_RIGHT);
            } else if (now - state_start_ms >= TURN_TIME_MS) {
                set_state(S1_DRIVE_CENTERED);
            }
            break;

        case S4_RECOVER_REVERSE:
            Motor_Drive(&MotorA, MOTOR_BACKWARD, RECOVERY_SPEED);
            Motor_Drive(&MotorB, MOTOR_BACKWARD, RECOVERY_SPEED);
            if (now - state_start_ms >= RECOVERY_TIME_MS) {
                stop_robot();
                set_state(next_state);
            }
            break;

        case S0_IDLE:
        case S5_STOP:
        default:
            stop_robot();
            break;
        }

        HAL_Delay(100);
    }
}
