#include "config.h"
#include "hal.h"
#include "motor.h"
#include "ultrasonic.h"
#include "esp_wifi.h"
#include "debug.h"

/* Helper to read ultrasonic sensor and send via WiFi */
void Send_Sensor_Data(void) {
    if (ESP_Wifi_IsConnected()) {
        uint32_t distance = Ultrasonic_Read_cm();
        ESP_Wifi_SendData((float)distance);
    }
}

int main(void)
{
    /* --- Initialization --- */
    System_Init();
    Debug_Init(115200);
    Motor_Init();
    Ultrasonic_Init();
    
    DBGLN("--- Wall Follower System Started ---");
    
    ESP_Wifi_Init(115200);
    if (ESP_Wifi_Connect()) {
        DBGLN("[System] WiFi Connected and Ready");
    } else {
        DBGLN("[System] WiFi Connection Failed - Proceeding in offline mode");
    }

    /* --- Main Loop --- */
    while (1) {
        /*
         * Note: Motor logic is preserved exactly as original.
         * Sensor data is sent before each movement segment.
         */
        
        Send_Sensor_Data();
        Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);
        Motor_Stop(&MotorB);
        HAL_Delay(2000);

        Send_Sensor_Data();
        Motor_Stop(&MotorA);
        Motor_Drive(&MotorB, MOTOR_FORWARD, BASE_SPEED);
        HAL_Delay(2000);

        Send_Sensor_Data();
        Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);
        Motor_Drive(&MotorB, MOTOR_FORWARD, BASE_SPEED);
        HAL_Delay(2000);

        Send_Sensor_Data();
        Motor_Stop(&MotorA);
        Motor_Stop(&MotorB);
        HAL_Delay(2000);
    }
}