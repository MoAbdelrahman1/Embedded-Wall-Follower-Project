# STM32F401RC Pin Configuration

## Motor Control Pin Layout

```
                    STM32F401RC
                    
┌─────────────────────────────────────────────────────────────────┐
│                                                                   │
│  Motor A (LEFT)           Motor B (RIGHT)                        │
│  ───────────────           ─────────────────                     │
│  PA0 ──[PWM]──→ EN1      PA5 ──[PWM]──→ EN2                     │
│  PA1 ──────────→ IN1      PA3 ──────────→ IN3                   │
│  PA2 ──────────→ IN2      PA4 ──────────→ IN4                   │
│                                                                   │
│  TIM5_CH1 (PA0)           TIM2_CH1 (PA5)                        │
│  10 kHz, 999 max          10 kHz, 999 max                       │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

## Pin Assignments Summary

| Pin  | Function    | Direction | Timer | Type    | Purpose         |
|------|-------------|-----------|-------|---------|-----------------|
| PA0  | EN1 (Motor A) | Output   | TIM5  | PWM     | Speed control   |
| PA1  | IN1 (Motor A) | Output   | —     | GPIO    | Forward/Backward|
| PA2  | IN2 (Motor A) | Output   | —     | GPIO    | Forward/Backward|
| PA3  | IN3 (Motor B) | Output   | —     | GPIO    | Forward/Backward|
| PA4  | IN4 (Motor B) | Output   | —     | GPIO    | Forward/Backward|
| PA5  | EN2 (Motor B) | Output   | TIM2  | PWM     | Speed control   |

## Motor Control Logic

### Motor A (Left) - PA0, PA1, PA2
```
     PA1    PA2    Result
    ─────  ─────  ────────────────
     1      0     Forward (CCW)
     0      1     Backward (CW)
     0      0     Stop
```

### Motor B (Right) - PA5, PA3, PA4
```
     PA3    PA4    Result
    ─────  ─────  ────────────────
     1      0     Forward (CW)
     0      1     Backward (CCW)
     0      0     Stop
```

## PWM Configuration

```
System Clock:  84 MHz
Timers Used:   TIM5 (Motor A), TIM2 (Motor B)
Prescaler:     84 (divides 84 MHz → 1 MHz counting)
ARR (Period):  999 (1 MHz / 1000 = 10 kHz frequency)

Speed Formula:
PWM Frequency = System Clock / (Prescaler × (ARR + 1))
              = 84 MHz / (84 × 1000) = 10 kHz

Duty Cycle:
Percent = (CCR / ARR) × 100
Example: CCR=600, ARR=999 → 600/999 = 60% duty
```

## Timing Diagram (Motor A Forward at 50% Speed)

```
PA0 (EN1)     ┌─────┐     ┌─────┐     ┌─────┐
PWM 50%       │     │     │     │     │     │
              └─────┘─────└─────┘─────└─────┘
              |-----|-----|  Period: 100 μs
              50 μs (50% on, 50% off)

PA1 (IN1)     ───────────────────────────────
              │ = 1 (High, Forward)

PA2 (IN2)     ─────────────────────────────
              │ = 0 (Low, Forward)
```

## Clock Tree

```
HSE Input (8 MHz)
    │
    └─→ PLL
         ├─ PLLM = 8     (8 MHz / 8 = 1 MHz)
         ├─ PLLN = 336   (1 MHz × 336 = 336 MHz)
         └─ PLLP = 4     (336 MHz / 4 = 84 MHz) ← System Clock
                          │
         ┌────────────────┼────────────────┐
         │                │                │
       APB2            APB1 Prescaler    HCLK
      84 MHz        (÷2) 42 MHz         84 MHz
         │                │                │
       ADC, SPI2      UART, SPI1       Core, Bus
              TIM5, TIM2 (on APB1 → 84 MHz)
```

## SysTick Configuration

```
Base Frequency: 84 MHz
SysTick Period: 1 ms = 0.001 seconds
Ticks per ms:   84,000 clock cycles

Result: HAL_GetTick() increments every 1 ms
Used for: Timing delays, control loop scheduling
```

## Hardware Connection Example (Typical)

```
┌──────────────────────┐
│  STM32F401RC Board   │
│  ─────────────────   │
│                      │
│  VCC ── [Motor A]    │ (Left Motor)
│  GND ── [Motor B]    │ (Right Motor)
│                      │
│  PA0  ──┬──→ EN1     │ PWM Speed
│  PA1  ──┼──→ IN1     │ Direction
│  PA2  ──┼──→ IN2     │ Direction
│         │            │
│  PA3  ──┼──→ IN3     │ Direction
│  PA4  ──┼──→ IN4     │ Direction
│  PA5  ──┴──→ EN2     │ PWM Speed
│                      │
└──────────────────────┘
         ▼
┌──────────────────────┐
│   H-Bridge Driver    │
│ (e.g., L298N or      │
│  similar motor       │
│  controller)         │
└──────────────────────┘
         ▼
┌──────────────────────┐
│   Motors (DC)        │
│  Left (Motor A)      │
│  Right (Motor B)     │
└──────────────────────┘
```

## Debugging: How to Check Motor Control

### Check PWM is running:
- Use oscilloscope on PA0 (Motor A) or PA5 (Motor B)
- Should see 10 kHz square wave
- Duty cycle changes with `Motor_SetSpeed()`

### Check Direction pins:
- Multimeter on PA1/PA2 (Motor A) or PA3/PA4 (Motor B)
- Should read 0V or 3.3V depending on direction

### Check SysTick:
- Add LED on unused GPIO
- Toggle it in `SysTick_Handler()`
- Should blink at known frequency

---

## Complete Pin Reference

| Port | Pin | Function        | Config Type | Init Value |
|------|-----|-----------------|-------------|------------|
| PA   | 0   | Motor A EN (PWM)| Alternate   | 0 V (PWM) |
| PA   | 1   | Motor A IN1     | Output      | 0 V       |
| PA   | 2   | Motor A IN2     | Output      | 0 V       |
| PA   | 3   | Motor B IN3     | Output      | 0 V       |
| PA   | 4   | Motor B IN4     | Output      | 0 V       |
| PA   | 5   | Motor B EN (PWM)| Alternate   | 0 V (PWM) |

---

All configuration is implemented in:
- **config.h** - Constants and pin definitions
- **drivers/motor.c** - GPIO & PWM initialization
- **drivers/hal.c** - System clock & SysTick setup
