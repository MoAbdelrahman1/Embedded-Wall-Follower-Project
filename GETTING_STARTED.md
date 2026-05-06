# 🤖 Wall Robot: Getting Started Guide

**This guide will get your project building in 5 minutes!**

---

## What You Have Now ✅

Your project is **fully set up** with:
- ✅ Modular code structure (HAL, Motor drivers, Main app)
- ✅ STM32F401RC6 configured for 84 MHz operation
- ✅ Low-level PWM motor control (NO Arduino libraries)
- ✅ SysTick timer for accurate timing
- ✅ PlatformIO configured with correct board settings

---

## Step 1: Open in VS Code (2 min)

1. **Open VS Code**
2. **File** → **Open Folder**
3. Navigate to: `c:\Users\moham\Documents\PlatformIO\Projects\wall_robot`
4. Click **Select Folder**
5. Wait ~10 seconds for PlatformIO to initialize

**You should see:**
- Green PlatformIO logo in left sidebar
- Project name "wall_robot" in explorer
- Files: config.h, hal.c, motor.c, main.c visible

---

## Step 2: Build the Project (1 min)

**Option A: GUI (Recommended)**
1. Click the **PlatformIO** icon (left sidebar, alien face)
2. Expand **"wall_robot"** section
3. Expand **"Build"**
4. Click the ✅ **checkmark** icon
5. Watch the terminal output

**Option B: Keyboard Shortcut**
1. Press `Ctrl + Shift + P`
2. Type: `PlatformIO: Build`
3. Press Enter

**What to expect:**
- Terminal shows: `Building in release mode`
- Compiles: hal.c → motor.c → main.c
- Final line: ✓ **Project compiled successfully**

---

## Step 3: What Each File Does (Reference)

| File | What it Does |
|------|--------------|
| **config.h** | All `#define` constants - pins, speeds, thresholds |
| **hal.c** | Initializes system clock to 84 MHz + SysTick timer |
| **motor.c** | Configures PWM timers and GPIO for motor control |
| **main.c** | Your application code (control loop goes here) |
| **platformio.ini** | Build configuration for STM32F401RC |

---

## Step 4: Understanding the Motor Control

### Motor Speed (PWM on PA0 and PA5)
```
Speed = 0     → Motor off
Speed = 600   → Base speed (60% power)
Speed = 999   → Maximum (100% power)
```

### Motor Direction (GPIO pins)
```
IN1 = 1, IN2 = 0  → Forward
IN1 = 0, IN2 = 1  → Backward
IN1 = 0, IN2 = 0  → Stop
```

### Example Code (Add to ControlLoop() in main.c)
```c
/* Drive left motor forward at base speed */
Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);

/* Drive right motor forward at reduced speed for turning */
Motor_Drive(&MotorB, MOTOR_FORWARD, 400);

/* Stop a motor */
Motor_Stop(&MotorA);
```

---

## Step 5: Next - Add Sensors (When Ready)

When you want to add distance sensors (HC-SR04), create:

1. **include/sensor.h**
```c
#ifndef SENSOR_H
#define SENSOR_H
void Sensor_Init(void);
uint16_t Sensor_GetDistance(void);  // returns cm
#endif
```

2. **drivers/sensor.c** - Implement measurement logic

3. **In main.c**, update `ControlLoop()`:
```c
void ControlLoop(void)
{
    uint16_t dist_front = Sensor_GetDistance();
    
    if (dist_front < FRONT_STOP_DIST) {
        Motor_Stop(&MotorA);
        Motor_Stop(&MotorB);
    } else {
        Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);
        Motor_Drive(&MotorB, MOTOR_FORWARD, BASE_SPEED);
    }
}
```

---

## Step 6: Upload to Hardware (When Ready)

When you have your **ST-Link programmer connected**:

1. Click PlatformIO → **Project Tasks** → **Upload**
2. Watch for: `Upload finished successfully`
3. Monitor serial output: PlatformIO → **Project Tasks** → **Monitor**

---

## Troubleshooting

### **Build fails with errors?**
1. Click **PlatformIO** → **Rebuild IntelliSense**
2. Try: **PlatformIO** → **Project Tasks** → **Clean**
3. Then build again

### **Can't see PlatformIO icon?**
1. PlatformIO IDE might still be installing
2. Reload VS Code: `Ctrl+Shift+P` → `Reload Window`

### **Build succeeds but upload fails?**
1. Check ST-Link is properly connected
2. Windows Device Manager should show "STM32" device
3. Try `PlatformIO: Upload (Verbose)` to see details

---

## File Locations

```
c:\Users\moham\Documents\PlatformIO\Projects\wall_robot\
├── platformio.ini          ← Build config (already correct)
├── ARCHITECTURE.md         ← Full technical documentation
├── QUICKSTART.md           ← VS Code setup guide
├── build.bat               ← Build script (optional)
├── include/
│   ├── config.h            ← Pin definitions
│   ├── hal.h               ← Hardware abstraction
│   └── motor.h             ← Motor interface
├── drivers/
│   ├── hal.c               ← Clock setup
│   └── motor.c             ← PWM + GPIO
└── src/
    └── main.c              ← Your code here!
```

---

## Quick Reference

### Motor Control Examples
```c
/* Forward */
Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);

/* Turn right (left faster) */
Motor_Drive(&MotorA, MOTOR_FORWARD, BASE_SPEED);
Motor_Drive(&MotorB, MOTOR_FORWARD, 300);

/* Stop */
Motor_Stop(&MotorA);

/* Backward */
Motor_Drive(&MotorA, MOTOR_BACKWARD, BASE_SPEED);
```

### Config Values
```c
BASE_SPEED = 600      // Normal driving speed
TURN_SPEED = 450      // Speed during turns
CONTROL_PERIOD_MS = 20 // Run control loop every 20ms
```

---

## ✅ Checklist: You're Ready When...

- [ ] PlatformIO IDE extension installed in VS Code
- [ ] Project folder opened in VS Code
- [ ] Build runs without errors (shows ✓ compiled successfully)
- [ ] You understand the file structure (config.h, hal.c, motor.c, main.c)
- [ ] You can modify motor speeds in config.h
- [ ] (Optional) You've connected ST-Link and uploaded firmware

---

## 🚀 You're All Set!

Your project is **ready to go**. Now you can:

1. **Modify control logic** in `ControlLoop()` (main.c)
2. **Tune motor speeds** in config.h
3. **Add sensors** (create drivers/sensor.c when ready)
4. **Upload to hardware** with ST-Link

---

**Questions? Check:**
- [ARCHITECTURE.md](ARCHITECTURE.md) - Full technical details
- [QUICKSTART.md](QUICKSTART.md) - VS Code specific setup
- [config.h](include/config.h) - Pin definitions and constants

**Ready to code? Open main.c and start implementing your control logic!**
