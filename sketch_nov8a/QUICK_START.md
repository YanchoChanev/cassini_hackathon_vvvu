# Quick Start Guide - Heart Rate & SpO2 Monitor

## 🔌 Hardware Connections

### MAX30102 Sensor → ESP32
```
VIN → 3.3V
GND → GND
SDA → GPIO 21
SCL → GPIO 22
```

### Grove Button → ESP32
```
SIG (Yellow) → GPIO 4
VCC (Red)    → 3.3V
GND (Black)  → GND
NC (White)   → Not connected
```

## 📤 Upload & Run

1. **Open Arduino IDE**
2. **Select Board:** ESP32 Dev Module
3. **Select Port:** Your ESP32 COM port
4. **Upload** the sketch
5. **Open Serial Monitor** at 115200 baud

## 📊 Reading the Output

### Log Format
```
Time | HR (bpm) | SpO2 (%) | Quality | Finger | Status
-----+----------+----------+---------+--------+--------
5s   | 72.3 bpm | 98.2%    | 85%     | YES    | ✓ Valid reading
```

### What Each Column Means
- **Time:** Seconds since startup
- **HR (bpm):** Heart rate in beats per minute (40-200 range)
- **SpO2 (%):** Blood oxygen saturation (70-100% range)
- **Quality:** Signal quality percentage (>30% needed for valid reading)
- **Finger:** Is finger detected on sensor?
- **Status:** Current measurement status

## 🎮 Button Controls

| Action | Function |
|--------|----------|
| **Single Press** | Pause/Resume logging |
| **Double Press** | Reset heart rate readings |
| **Long Press (2s)** | Activate SOS mode |

## 📝 How to Use

1. **Place finger** on MAX30102 sensor
2. **Press firmly** but not too hard
3. **Keep still** for accurate readings
4. **Wait 1-2 seconds** for first reading
5. **Look for** "✓ Valid reading" status

## ⚠️ Status Messages

| Message | Meaning | Action |
|---------|---------|--------|
| **Place finger on sensor** | No finger detected | Place finger on sensor |
| **Collecting data...** | Gathering samples | Wait 1 second |
| **Poor signal quality** | Signal too weak | Adjust finger pressure/position |
| **✓ Valid reading** | Good measurement | Reading is accurate |

## 🔧 Quick Troubleshooting

### Problem: No sensor detected
- ✅ Check I2C wiring (SDA=21, SCL=22)
- ✅ Verify 3.3V power
- ✅ Check connections are secure

### Problem: No finger detected
- ✅ Press finger more firmly
- ✅ Clean sensor surface
- ✅ Try different finger

### Problem: Poor signal quality
- ✅ Keep finger completely still
- ✅ Adjust finger pressure
- ✅ Avoid bright ambient light
- ✅ Wait 30 seconds for warmup

### Problem: Erratic readings
- ✅ Keep finger still
- ✅ Double-press button to reset
- ✅ Check for loose connections

## 📈 Expected Values

### Normal Resting Heart Rate
- **Adults:** 60-100 BPM
- **Athletes:** 40-60 BPM
- **Children:** 70-100 BPM

### Normal SpO2 Levels
- **Healthy:** 95-100%
- **Acceptable:** 90-95%
- **Low:** <90% (seek medical attention)

## 💡 Tips for Best Results

1. **Finger Placement:** Use index or middle finger
2. **Pressure:** Firm but comfortable
3. **Position:** Finger flat on sensor, covering both LEDs
4. **Movement:** Stay completely still
5. **Environment:** Avoid bright lights
6. **Warmup:** Wait 30 seconds after power-on
7. **Clean:** Keep sensor surface clean

## 🚨 Important Notes

⚠️ **This device is for educational purposes only**
⚠️ **Not for medical diagnosis or treatment**
⚠️ **Consult healthcare professional for medical concerns**

## 📞 Need Help?

Check the full documentation:
- **HEART_RATE_MONITOR_GUIDE.md** - Complete guide
- **services/README.md** - Service documentation
- **drivers/README.md** - Driver documentation

