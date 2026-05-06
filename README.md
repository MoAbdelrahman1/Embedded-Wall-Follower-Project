# 📖 Project Documentation Index

Welcome to the **Wall Robot STM32F401RC Project**!

This document guides you through the project structure and documentation.

---

## 🚀 Start Here

**First time? Read this first:**
- [GETTING_STARTED.md](GETTING_STARTED.md) ⭐ **Start here!** (5 min read)
  - Quick VS Code setup
  - First build in 2 minutes
  - Basic overview of what each file does

---

## 📚 Detailed Documentation

### Architecture & Design
- [ARCHITECTURE.md](ARCHITECTURE.md) - Complete technical architecture
  - Hardware pin configuration
  - Project structure explanation
  - Motor control system design
  - Clock configuration details
  - Step-by-step setup instructions

### Quick References
- [PIN_CONFIG.md](PIN_CONFIG.md) - Hardware pin assignments & diagrams
  - Pin layout visualization
  - PWM configuration details
  - Motor control logic tables
  - Clock tree diagram
  - Debugging guides

### Setup Guides
- [QUICKSTART.md](QUICKSTART.md) - VS Code + PlatformIO installation
  - Extension installation steps
  - Common build issues & fixes
  - Upload & monitoring

### Configuration
- [include/config.h](include/config.h) - All #define constants
  - Motor pin definitions
  - PWM settings (frequency, max speed)
  - Control loop timing
  - Sensor thresholds

---

## 📁 Project File Structure

```
wall_robot/
├── 📄 platformio.ini          ← Build configuration (STM32F401RC)
│
├── 📚 Documentation/
│   ├── README.md              ← This file
│   ├── GETTING_STARTED.md     ⭐ Start here!
│   ├── ARCHITECTURE.md        ← Full technical design
│   ├── QUICKSTART.md          ← VS Code setup
│   └── PIN_CONFIG.md          ← Hardware pin reference
│
├── 🔧 include/                ← Header files
│   ├── config.h               ← Pin #defines and constants
│   ├── hal.h                  ← Hardware abstraction layer
│   └── motor.h                ← Motor control interface
│
├── 💾 drivers/                ← Low-level driver code
│   ├── hal.c                  ← System init, clock, SysTick
│   └── motor.c                ← PWM & GPIO motor control
│
├── 🎯 src/                    ← Application code
│   └── main.c                 ← Your code goes here!
│
├── 🧪 test/                   ← Unit tests (optional)
│
├── 📦 lib/                    ← External libraries (if needed)
│
└── 🔨 build.bat               ← Helper build script
```

---

## 🎯 What Each File Does

### Core Files

| File | Purpose | Status |
|------|---------|--------|
| **config.h** | Motor pins, PWM settings, timing constants | ✅ Complete |
| **hal.c** | System clock (84 MHz), SysTick timer | ✅ Complete |
| **motor.c** | PWM timers, GPIO control for motors | ✅ Complete |
| **main.c** | Application entry point, control loop | ✅ Template ready |

### Documentation Files

| File | Purpose | For Whom |
|------|---------|----------|
| **GETTING_STARTED.md** | 5-min quick start | Everyone first |
| **ARCHITECTURE.md** | Full technical design | Advanced users |
| **QUICKSTART.md** | VS Code setup | IDE users |
| **PIN_CONFIG.md** | Pin reference & diagrams | Hardware debugging |

---

## ⚡ Quick Start (30 seconds)

1. **Open VS Code** → **File** → **Open Folder** → Select this folder
2. **PlatformIO icon** (left sidebar) → **Build** → Click ✅
3. Watch terminal: should see ✓ **Project compiled successfully**

**Done!** Your project builds. Now read [GETTING_STARTED.md](GETTING_STARTED.md)

---

## 🛠️ Next Steps

### Phase 1: Understand (Now)
- [ ] Read [GETTING_STARTED.md](GETTING_STARTED.md)
- [ ] Look at [ARCHITECTURE.md](ARCHITECTURE.md) for overview
- [ ] Check [PIN_CONFIG.md](PIN_CONFIG.md) for hardware details
- [ ] Review [include/config.h](include/config.h)

### Phase 2: Build (When You're Ready)
- [ ] Connect ST-Link programmer
- [ ] Upload firmware via PlatformIO
- [ ] Monitor serial output (115200 baud)
- [ ] Verify motors respond

### Phase 3: Add Sensors (Next Feature)
- [ ] Create **drivers/sensor.c** for HC-SR04
- [ ] Create **include/sensor.h** interface
- [ ] Update `ControlLoop()` in main.c
- [ ] Implement wall-following algorithm

### Phase 4: Optimize (After Testing)
- [ ] Tune motor speeds in config.h
- [ ] Adjust PID controller (KP value)
- [ ] Optimize sensor thresholds
- [ ] Add debug logging

---

## 📋 Hardware Checklist

Before uploading, make sure you have:

- [ ] **STM32F401RC board** (or similar STM32F4 variant)
- [ ] **ST-Link programmer** (USB ST-Link v2 or similar)
- [ ] **Motor driver** (L298N, DRV8833, or equivalent H-bridge)
- [ ] **Two DC motors** (3-6V, appropriate for your robot)
- [ ] **Power supply** for motors (separate from ST-Link)
- [ ] **USB cable** to connect ST-Link to PC
- [ ] **Jumper wires** to connect board to motor driver

---

## 🔍 Troubleshooting

### Can't build?
1. Check PlatformIO is installed
2. Read [QUICKSTART.md](QUICKSTART.md#common-issues--fixes)
3. Try: **PlatformIO** → **Project Tasks** → **Clean** then **Build**

### PlatformIO icon not showing?
1. Reload VS Code: `Ctrl+Shift+P` → `Reload Window`
2. Reinstall PlatformIO IDE extension

### Upload fails?
1. Check ST-Link is connected via USB
2. Device Manager should show "STM32" device
3. Try: `PlatformIO: Upload (Verbose)` for details

### Motors don't respond?
1. Check pin connections match [PIN_CONFIG.md](PIN_CONFIG.md)
2. Verify motor driver power supply
3. Use multimeter to check PA0-PA5 voltages

---

## 📚 Documentation Hierarchy

**For different users:**

```
├── 👤 New User
│   └─→ GETTING_STARTED.md (5 min)
│       └─→ QUICKSTART.md (10 min)
│
├── 👨‍💻 Developer
│   └─→ ARCHITECTURE.md (20 min)
│       └─→ PIN_CONFIG.md (10 min)
│           └─→ Source code (drivers/*, include/*)
│
└── 🔧 Hardware Engineer
    └─→ PIN_CONFIG.md (hardware detail)
        └─→ config.h (register settings)
            └─→ drivers/hal.c, motor.c (implementation)
```

---

## 🎓 Learning Path

1. **Hour 1: Setup**
   - Read GETTING_STARTED.md
   - Get project building in VS Code
   - Understand file structure

2. **Hour 2: Hardware**
   - Read ARCHITECTURE.md
   - Study PIN_CONFIG.md
   - Review main.c control loop

3. **Hour 3: Development**
   - Connect hardware
   - Upload firmware
   - Test motor control
   - Start implementing wall-following

4. **Hour 4+: Optimization**
   - Add sensors
   - Implement algorithms
   - Tune parameters
   - Debug with serial output

---

## 💡 Pro Tips

- **IntelliSense not working?** → PlatformIO → Rebuild IntelliSense
- **Frequent rebuilds?** → Modify only main.c first to save compile time
- **Need debugging?** → Add UART driver and use printf()
- **Motor not responding?** → Check .pio/build/ output files exist

---

## 📞 Support

**Issue with build?**
→ Check [QUICKSTART.md#common-issues--fixes](QUICKSTART.md)

**Need hardware reference?**
→ See [PIN_CONFIG.md](PIN_CONFIG.md)

**Want architectural overview?**
→ Read [ARCHITECTURE.md](ARCHITECTURE.md)

**Don't know where to start?**
→ Begin with [GETTING_STARTED.md](GETTING_STARTED.md) ⭐

---

## ✅ You're All Set!

Everything is configured and ready to go. Your next step:

**→ Read [GETTING_STARTED.md](GETTING_STARTED.md) (5 minutes)**

Then you'll:
1. Build the project ✓
2. Upload to hardware ✓
3. Implement your wall-following robot ✓

---

**Project created: 2024**
**Framework: STM32CubeMX LL (Low-Level Drivers)**
**Board: STM32F401RC6 @ 84 MHz**
**Toolchain: arm-none-eabi-gcc via PlatformIO**
