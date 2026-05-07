#pragma once
#include <stdint.h>
#include <stdbool.h>

void ESP_Wifi_Init(uint32_t baud);
bool ESP_Wifi_Connect(void);
bool ESP_Wifi_SendData(float distance); // Adapted to send distance for wall follower
bool ESP_Wifi_IsConnected(void);
void ESP_Wifi_Reconnect(void);
