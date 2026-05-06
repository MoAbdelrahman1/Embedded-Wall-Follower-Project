# Wall Robot Project - Architecture Guide

## Project Overview
This is a **low-level STM32F401RC6 wall-following robot** using:
- **Microcontroller**: STM32F401RC6 (84 MHz, STM32F401 Line)
- **Motor Driver**: Dual H-bridge with PWM speed control
- **Framework**: STM32CubeMX LL (Low-Level) Drivers - **NO Arduino libraries**
- **IDE**: VS Code + PlatformIO

## Hardware Pin Configuration

### Motor A (Left Motor)
| Function | Pin  | Purpose         |
|----------|------|-----------------|
| EN1      | PA0  | PWM Speed (TIM5_CH1) |
| IN1      | PA1  | Direction bit 1      |
| IN2      | PA2  | Direction bit 2      |

### Motor B (Right Motor)
| Function | Pin  | Purpose         |
|----------|------|-----------------|
| EN2      | PA5  | PWM Speed (TIM2_CH1) |
| IN3      | PA3  | Direction bit 1      |
| IN4      | PA4  | Direction bit 2      |

### Sensors (TODO)
- Front distance sensor: (HC-SR04 or similar)
- Left/Right distance sensors: (For wall following)

---

## Project Structure

```
wall_robot/
├── platformio.ini          # PlatformIO configuration
├── include/
│   ├── config.h            # All defines (pins, timing, thresholds)
│   ├── hal.h               # Hardware abstraction layer
│   └── motor.h             # Motor control interface
├── drivers/
│   ├── hal.c               # System init, clock, SysTick
│   └── motor.c             # Motor PWM & GPIO implementation
├── src/
│   └── main.c              # Application entry point
├── lib/                    # External libraries (if needed)
├── modules/                # Sensor modules (sensors.c, etc)
└── test/                   # Unit tests (optional)
```

---

## Step-by-Step Setup in VS Code

### 1. **Install Required Extensions**
Open VS Code and install:
- **PlatformIO IDE** (by PlatformIO)
- **C/C++ IntelliSense** (by Microsoft)
- **Cortex-Debug** (for STM32 debugging)

### 2. **Verify Project Structure**
You should already have:
```
✓ platformio.ini
✓ include/config.h, hal.h, motor.h
✓ drivers/hal.c, motor.c
✓ src/main.c
```

### 3. **Verify platformio.ini**
Your `platformio.ini` should look like:
```ini
[env:genericSTM32F401RC]
platform        = ststm32
board           = genericSTM32F401RC
framework       = stm32cube
upload_protocol = stlink
monitor_speed   = 115200
build_flags     =
    -DSTM32F401xC
    -DUSE_FULL_LL_DRIVER
    -DHSE_VALUE=8000000
```

### 4. **Build the Project**
1. Open PlatformIO Home (click PlatformIO icon in sidebar)
2. Click **"Project Tasks"** → **"Build"**
3. Watch the build output - should show **`✓ Project compiled successfully`**

### 5. **Troubleshooting Build Errors**

#### Error: "stm32f4xx_ll_*.h not found"
- **Fix**: Ensure `framework = stm32cube` and `platform = ststm32` in platformio.ini

#### Error: "Undefined reference to `SysTick_Handler`"
- **Fix**: This is defined in `drivers/hal.c` - ensure it's included in build

#### Error: "Multiple definitions of main"
- **Fix**: Make sure you only have ONE main() function (should be in src/main.c)

### 6. **Programming the STM32**
1. Connect ST-Link programmer to your board
2. In PlatformIO: **Project Tasks** → **Upload**
3. Monitor output with: **Project Tasks** → **Monitor**

---

## How the Motor Control Works

### Motor Speed Control (PWM)
- **Timer**: TIM5 (Motor A), TIM2 (Motor B)
- **Frequency**: 10 kHz (PWM_MAX = 999)
- **Prescaler**: 84 (1 MHz counting)
- **Speed ranges**:
  - `0` = Stop
  - `600` = Base speed (config.h)
  - `999` = Maximum

### Motor Direction Control (H-Bridge Logic)
```
IN1 | IN2 | Direction
----|-----|----------
 1  |  0  | Forward
 0  |  1  | Backward
 0  |  0  | Stop
 1  |  1  | Stop (short circuit protection)
```

### Example Usage in main.c
```c
/* Initialize */
HAL_Init();      // System clock + SysTick
Motor_Init();    // GPIO + PWM setup

/* Drive forward at base speed */
Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);

/* Turn left (slow right motor) */
Motor_Drive(&MotorB, MOTOR_FORWARD, 300);

/* Stop */
Motor_Stop(&MotorA);
```

---

## Adding Sensor Support

To add distance sensors (HC-SR04), create:

1. **include/sensor.h**
```c
#ifndef SENSOR_H
#define SENSOR_H
#include <stdint.h>

void Sensor_Init(void);
uint16_t Sensor_GetDistance(uint8_t sensor_id);  // Returns cm

#endif
```

2. **drivers/sensor.c** - Implement TRIG/ECHO timing using timers or GPIO

---

## Clock Configuration

The system clock is configured to:
- **HSE Input**: 8 MHz (from external crystal)
- **PLL Multiplier**: 336
- **PLL Divider**: 4
- **System Clock**: 84 MHz
- **APB1 Clock**: 42 MHz (timers, UART, SPI)
- **APB2 Clock**: 84 MHz (ADC, more timers)
- **SysTick**: 1 ms interrupts

---

## Common Next Steps

1. **Add Serial Communication** - For debugging/logging
   - Create `drivers/uart.c`
   - Initialize UART1 on PA9/PA10

2. **Add Sensor Support** - Distance measurement
   - Create `drivers/sensor.c`
   - Implement HC-SR04 timing

3. **Implement Wall-Following Algorithm**
   - Update `ControlLoop()` in main.c
   - Use PID controller (config.h has KP constant)

4. **Add LED Debugging** - Status indication
   - Use a GPIO pin for blinking/status

---

## Building & Debugging

### Full Rebuild
```bash
PlatformIO: Project Tasks → Clean
PlatformIO: Project Tasks → Build
```

### Upload & Monitor
```bash
PlatformIO: Project Tasks → Upload
PlatformIO: Project Tasks → Monitor  (115200 baud)
```

### Debugging with ST-Link
1. Install **Cortex-Debug** extension
2. Create `.vscode/launch.json`
3. Click the Debug icon and start debugger

---

## File Summary

| File | Purpose |
|------|---------|
| **config.h** | Pin defines, PWM settings, thresholds, timing |
| **hal.h / hal.c** | System clock, SysTick, timing functions |
| **motor.h / motor.c** | Low-level motor control (PWM, GPIO) |
| **main.c** | Application logic, control loop |
| **platformio.ini** | Build configuration, board settings |

---

## Key Defines (config.h)

```c
MOTOR_A_EN_PIN   0    // PA0 - PWM
MOTOR_A_IN1_PIN  1    // PA1 - Direction
MOTOR_A_IN2_PIN  2    // PA2 - Direction

MOTOR_B_EN_PIN   5    // PA5 - PWM
MOTOR_B_IN3_PIN  3    // PA3 - Direction
MOTOR_B_IN4_PIN  4    // PA4 - Direction

PWM_MAX          999  // 10 kHz at 84 MHz
BASE_SPEED       600  // Default duty cycle
CONTROL_PERIOD_MS 20  // 50 Hz control loop
```

---

**Ready to build? Run: PlatformIO → Build**
