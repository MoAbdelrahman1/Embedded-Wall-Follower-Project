#include "esp_wifi.h"
#include "config.h"
#include "debug.h"
#include "hal.h"
#include "stm32f401xc.h"
#include <stdio.h>
#include <string.h>

static bool _connected = false;

/* Using USART6 on PA11 (TX) and PA12 (RX) */
static void USART6_Init(uint32_t baud) {
    /* Enable GPIOA and USART6 clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART6EN;

    /* PA11 (TX), PA12 (RX) -> AF8 (USART6) */
    GPIOA->MODER &= ~((3U << 22) | (3U << 24));
    GPIOA->MODER |=  ((2U << 22) | (2U << 24));
    GPIOA->AFR[1] &= ~((0xFU << 12) | (0xFU << 16));
    GPIOA->AFR[1] |=  ((8U << 12) | (8U << 16));

    /* 
     * USART6 clock = APB2 = 84MHz (assuming 84MHz System Clock)
     * For 115200 baud: 84e6 / (16 * 115200) = 45.57 -> 45:9 
     */
    if (baud == 115200) {
        USART6->BRR = (45U << 4) | 9U;
    } else if (baud == 9600) {
        USART6->BRR = (546U << 4) | 14U;
    }
    
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
                if (strstr(buf, token)) {
                    DBGF("[ESP] << %s\n", buf);
                    return true;
                }
            } else {
                memmove(buf, buf + 256, 256);
                pos = 256;
            }
        }
    }
    DBGF("[ESP] Timeout waiting for '%s'\n", token);
    return false;
}

static bool _connectWifi(void) {
    DBGLN("[WiFi] Connecting...");
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
    _send(cmd);

    if (!_waitFor("WIFI GOT IP", 20000)) {
        DBGLN("[WiFi] Failed — wrong SSID/password or weak signal");
        return false;
    }
    DBGLN("[WiFi] Connected, got IP");
    return true;
}

static bool _openTCP(void) {
    DBGLN("[TCP] Opening connection...");
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", SERVER_IP, SERVER_PORT);
    _send(cmd);

    if (!_waitFor("CONNECT", 5000)) {
        DBGLN("[TCP] Failed — is receiver.py running on your laptop?");
        return false;
    }
    DBGF("[TCP] Connected to %s:%s\n", SERVER_IP, SERVER_PORT);
    return true;
}

void ESP_Wifi_Init(uint32_t baud) {
    USART6_Init(baud);
    HAL_Delay(100);
}

bool ESP_Wifi_Connect(void) {
    HAL_Delay(1500);

    _send("AT\r\n");
    if (!_waitFor("OK", 3000)) {
        DBGLN("[ESP] No response to AT — check wiring");
        return false;
    }
    DBGLN("[ESP] Module alive");

    _send("AT+RST\r\n");
    _waitFor("ready", 5000);

    _send("AT+CWMODE=1\r\n");
    _waitFor("OK", 2000);

    if (!_connectWifi()) return false;
    if (!_openTCP())     return false;

    _connected = true;
    return true;
}

bool ESP_Wifi_SendData(float distance) {
    char payload[64];
    int  len = snprintf(payload, sizeof(payload), "{\"distance\":%.2f}\n", distance);

    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", len);
    _send(cmd);

    if (!_waitFor(">", 2000)) {
        DBGLN("[ESP] No '>' prompt — reconnecting...");
        _connected = false;
        return false;
    }

    _send(payload);

    if (_waitFor("SEND OK", 3000)) {
        DBGF("[ESP] Sent → %s", payload);
        return true;
    }

    DBGLN("[ESP] Send failed");
    _connected = false;
    return false;
}

bool ESP_Wifi_IsConnected(void) {
    return _connected;
}

void ESP_Wifi_Reconnect(void) {
    DBGLN("[ESP] Reconnecting...");
    _connected = false;
    HAL_Delay(1000);
    ESP_Wifi_Connect();
}
