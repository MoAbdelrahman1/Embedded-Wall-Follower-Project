# VS Code + PlatformIO Quick Start

## ⚡ Installation & First Build

### Step 1: Install VS Code Extension
1. Open VS Code
2. Click the **Extensions** icon (left sidebar)
3. Search for **"PlatformIO IDE"**
4. Click **Install** (by PlatformIO)
5. Reload VS Code when prompted

### Step 2: Open Your Project
1. File → Open Folder
2. Select: `c:\Users\moham\Documents\PlatformIO\Projects\wall_robot`
3. Wait for PlatformIO to index (you'll see "PlatformIO" in bottom taskbar)

### Step 3: Verify Your Project
Left sidebar should show a **PlatformIO** icon. Click it, and you should see:
- **STM32F401RC** board selected
- **ststm32** platform
- **stm32cube** framework

### Step 4: Build
**Option A - GUI (Easiest)**
1. Click **PlatformIO** icon in left sidebar
2. Expand **"wall_robot"** → **"Build"**
3. Click the **checkmark icon** next to "Build"
4. Watch terminal output

**Option B - Command Palette**
1. Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
2. Type: `PlatformIO: Build`
3. Press Enter

### Step 5: Check for Errors
After building, you'll see one of these:
- ✅ **SUCCESS**: `✓ Project compiled successfully`
- ❌ **ERROR**: Red text in terminal showing what's wrong

---

## 📦 Project Files Created

Your project now has these structure:

```
✓ include/
  ├── config.h        ← Pin definitions, constants
  ├── hal.h           ← Hardware initialization
  └── motor.h         ← Motor control interface

✓ drivers/
  ├── hal.c           ← System clock, SysTick setup
  └── motor.c         ← PWM + GPIO implementation

✓ src/
  └── main.c          ← Your application code

✓ platformio.ini      ← Build configuration (already configured)
✓ ARCHITECTURE.md     ← Full technical documentation
```

---

## 🔧 Common Issues & Fixes

### Issue 1: "platformio.ini not found"
**Fix**: Make sure VS Code opens the folder: `wall_robot/` (NOT a parent)

### Issue 2: Build fails with "stm32f4xx_ll_*.h: No such file"
**Fix**: Check platformio.ini has:
```ini
framework = stm32cube
platform = ststm32
```

### Issue 3: "undefined reference to `SysTick_Handler`"
**Fix**: Already included in `drivers/hal.c` - just rebuild

### Issue 4: Port not found when uploading
**Solution**:
1. Connect ST-Link to PC
2. Check Device Manager (Windows) for "STM32" device
3. Reinstall ST-Link drivers if needed

---

## 🚀 First Build Checklist

- [ ] PlatformIO IDE extension installed
- [ ] Project folder opened in VS Code
- [ ] Saw "PlatformIO" appear in sidebar
- [ ] All files visible in Explorer (config.h, hal.c, motor.c, main.c)
- [ ] Build runs without errors
- [ ] .pio/ build folder created

---

## 📤 Upload to Hardware

When ready to program your STM32:

1. **Connect ST-Link programmer** to your development board
2. **Click PlatformIO** → **Project Tasks** → **Upload**
3. **Check the monitor**: PlatformIO → Project Tasks → Monitor (115200 baud)

---

## 🔍 What's Happening Under the Hood

When you hit **Build**:

1. PlatformIO invokes **arm-none-eabi-gcc** (C compiler)
2. Compiles **hal.c** (system clock, interrupts)
3. Compiles **motor.c** (PWM timer configuration)
4. Compiles **main.c** (your application)
5. Includes STM32CubeMX LL drivers (from ststm32 package)
6. Generates **firmware.elf** (executable image)
7. Converts to **.bin** file for ST-Link upload

---

## 📝 Next Steps

1. **Try the Build** - Should succeed with no errors
2. **Check [ARCHITECTURE.md](ARCHITECTURE.md)** - Full technical details
3. **Modify main.c** - Add your control logic here
4. **Add sensors** - Create `drivers/sensor.c` for HC-SR04
5. **Implement algorithm** - Wall-following logic in `ControlLoop()`

---

## 💡 Pro Tips

- **Intellisense not working?** → Click PlatformIO icon → **Rebuild Intellisense**
- **Want to see detailed build?** → Check `.pio/build/` folder
- **Serial monitor issues?** → Right-click → **Change Port** or restart VS Code
- **Random errors after download?** → Try **Clean Build** first

---

**Ready? Press `Ctrl+Shift+P` → `PlatformIO: Build` → Go!**
