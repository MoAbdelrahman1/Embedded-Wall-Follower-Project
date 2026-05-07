#include "config.h"
#include "hal.h"
#include "motor.h"
#include "ultrasonic.h"
#include "debug.h"

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
    
    ESP_Wifi_Init(115200);
    if (ESP_Wifi_Connect()) {
        DBGLN("[System] WiFi Connected and Ready");
    } else {
        DBGLN("[System] WiFi Connection Failed - Proceeding in offline mode");
    }
    

    DBGLN("FSM Start");

    /* Start immediately (no start button) */
    set_state(S1_FOLLOW_WALL);

    while (1) {
        uint32_t now = HAL_GetTick();

        /* Read sensors */
        uint32_t front = Ultrasonic_Read_cm(FRONT_TRIG, FRONT_ECHO);
        uint32_t left  = Ultrasonic_Read_cm(LEFT_TRIG,  LEFT_ECHO);
        uint32_t right = Ultrasonic_Read_cm(RIGHT_TRIG, RIGHT_ECHO);

        // DBGF("F:%lu L:%lu R:%lu\n", front, left, right);

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
                    set_state(S2_TURN_LEFT); /* prefer left */
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
            /* slow forward search */
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

        HAL_Delay(100); /* slow sensor prints */
    }
}