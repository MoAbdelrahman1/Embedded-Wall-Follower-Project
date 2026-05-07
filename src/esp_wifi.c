#include "esp_wifi.h"
#include "config.h"
#include "debug.h"
#include "hal.h"
#include "stm32f401xc.h"
#include <stdio.h>
#include <string.h>

static bool _connected = false;

static void USART6_Init(uint32_t baud) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    GPIOA->MODER &= ~((3U << 22) | (3U << 24));
    GPIOA->MODER |=  ((2U << 22) | (2U << 24));
    GPIOA->AFR[1] &= ~((0xFU << 12) | (0xFU << 16));
    GPIOA->AFR[1] |=  ((8U << 12) | (8U << 16));

    uint32_t pclk = 16000000; 
    USART6->BRR = pclk / baud;
    USART6->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static void _send(const char* cmd) {
    for (int i = 0; cmd[i] != '\0'; i++) {
        while (!(USART6->SR & USART_SR_TXE));
        USART6->DR = (uint8_t)cmd[i];
    }
}

static bool _waitFor(const char* token, uint32_t timeout_ms) {
    char buf[512];
    int pos = 0;
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < timeout_ms) {
        if (USART6->SR & USART_SR_RXNE) {
            char c = (char)USART6->DR;
            if (pos < sizeof(buf) - 1) {
                buf[pos++] = c;
                buf[pos] = '\0';
                if (strstr(buf, token)) return true;
            } else {
                memmove(buf, buf + 256, 256);
                pos = 256;
            }
        }
    }
    return false;
}

static bool _setupAP(void) {
    DBGLN("[AP] Setting up Robot Hotspot...");
    _send("AT+CWMODE=2\r\n");
    _waitFor("OK", 2000);

    char cmd[128];
    // AT+CWSAP="SSID","PASS",channel,encryption
    // Encryption 3 = WPA2_PSK
    snprintf(cmd, sizeof(cmd), "AT+CWSAP=\"%s\",\"%s\",5,3\r\n", ROBOT_SSID, ROBOT_PASS);
    _send(cmd);
    
    if (_waitFor("OK", 5000)) {
        DBGF("[AP] Hotspot '%s' is Ready!\n", ROBOT_SSID);
        return true;
    }
    DBGLN("[AP] Failed to start hotspot");
    return false;
}

static bool _openTCP(void) {
    DBGF("[TCP] Connecting to Laptop at %s...\n", SERVER_IP);
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", SERVER_IP, SERVER_PORT);
    _send(cmd);

    if (_waitFor("CONNECT", 3000)) {
        DBGLN("[TCP] Connected to Laptop!");
        return true;
    }
    return false;
}

void ESP_Wifi_Init(uint32_t baud) {
    USART6_Init(baud);
    HAL_Delay(100);
}

bool ESP_Wifi_Connect(void) {
    HAL_Delay(1000);
    _send("AT\r\n");
    if (!_waitFor("OK", 2000)) return false;

    if (!_setupAP()) return false;
    
    DBGLN("[System] Please connect your laptop to the Robot's WiFi now.");
    DBGLN("[System] Waiting for TCP server...");

    // Try to connect to laptop multiple times
    for (int i = 0; i < 10; i++) {
        if (_openTCP()) {
            _connected = true;
            return true;
        }
        HAL_Delay(2000);
    }

    DBGLN("[System] No laptop detected. Starting in offline mode.");
    return false;
}

bool ESP_Wifi_SendData(float distance) {
    if (!_connected) return false;
    char payload[64];
    int len = snprintf(payload, sizeof(payload), "{\"distance\":%.2f}\n", distance);
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", len);
    _send(cmd);
    if (!_waitFor(">", 1000)) {
        _connected = false;
        return false;
    }
    _send(payload);
    if (_waitFor("SEND OK", 2000)) return true;
    _connected = false;
    return false;
}

bool ESP_Wifi_IsConnected(void) { return _connected; }
void ESP_Wifi_Reconnect(void) { _connected = false; ESP_Wifi_Connect(); }