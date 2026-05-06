#include "config.h"
#include "hal.h"
#include "motor.h"

int main(void)
{
    System_Init();
    Motor_Init();

    while (1) {
        Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);
        Motor_Stop(&MotorB);
        HAL_Delay(2000);

        Motor_Stop(&MotorA);
        Motor_Drive(&MotorB, MOTOR_FORWARD, BASE_SPEED);
        HAL_Delay(2000);

        Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);
        Motor_Drive(&MotorB, MOTOR_FORWARD, BASE_SPEED);
        HAL_Delay(2000);

        Motor_Stop(&MotorA);
        Motor_Stop(&MotorB);
        HAL_Delay(2000);
    }
}