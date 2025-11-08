# Heart Rate & SpO2 Monitor - Complete Guide

## Overview
This application monitors heart rate (BPM) and blood oxygen saturation (SpO2) using the MAX30102 sensor on ESP32-WROOM-32D, with a Grove button for control.

## Hardware Setup

### Components Required
1. **ESP32-WROOM-32D** development board
2. **MAX30102** heart rate and SpO2 sensor
3. **Grove Button** (from Grove Beginner Kit)
4. Jumper wires or Grove cables

### Wiring Connections

#### MAX30102 Sensor (I2C)
```
MAX30102          ESP32-WROOM-32D
--------          ----------------
VIN          →    3.3V
GND          →    GND
SDA          →    GPIO 21
SCL          →    GPIO 22
```

#### Grove Button
```
Grove Button      ESP32-WROOM-32D
------------      ----------------
SIG (Yellow) →    GPIO 4
NC (White)   →    (not connected)
VCC (Red)    →    3.3V
GND (Black)  →    GND
```

**Important Notes:**
- Use **3.3V** for both sensors (ESP32 is 3.3V logic)
- Grove button is configured as **active HIGH** (pressed = HIGH)
- Ensure good I2C connections with proper pull-up resistors (usually built-in on sensor boards)

## Software Architecture

### Components

1. **MAX30102_Driver** (Driver Layer)
   - Low-level sensor communication
   - FIFO data reading
   - Configuration management

2. **HeartRate_Service** (Service Layer)
   - Heart rate calculation using peak detection
   - SpO2 calculation using RED/IR ratio
   - Signal quality assessment
   - Finger detection

3. **SOSButton_Driver** (Driver Layer)
   - Button debouncing
   - Long press detection (2 seconds)
   - Double press detection (400ms)
   - Event callbacks

## Features

### Heart Rate Monitoring
- **Range:** 40-200 BPM
- **Algorithm:** Peak detection on IR signal
- **Smoothing:** Exponential moving average
- **Update Rate:** Every 1 second

### SpO2 Monitoring
- **Range:** 70-100%
- **Algorithm:** RED/IR ratio calculation
- **Formula:** SpO2 = 110 - 25*R (where R = (RED_AC/RED_DC)/(IR_AC/IR_DC))
- **Smoothing:** Exponential moving average

### Signal Quality
- **Metrics:**
  - Signal strength (IR DC value)
  - Signal variation (IR AC value)
  - Signal stability (standard deviation)
- **Score:** 0-100%
- **Threshold:** >30% for valid reading

### Button Controls
- **Single Press:** Toggle logging on/off
- **Long Press (2s):** Activate SOS mode (emergency signal)
- **Double Press:** Reset heart rate readings

## Output Format

### Log Header
```
LOG FORMAT:
Time | HR (bpm) | SpO2 (%) | Quality | Finger | Status
-----+----------+----------+---------+--------+--------
```

### Example Output
```
5s | 72.3 bpm | 98.2% | 85% | YES | ✓ Valid reading
6s | 73.1 bpm | 98.1% | 87% | YES | ✓ Valid reading
7s | --- | ---% | 12% | NO  | Place finger on sensor
8s | 71.8 bpm | 97.9% | 45% | YES | Poor signal quality
```

### Status Messages
- **"Place finger on sensor"** - No finger detected
- **"Collecting data..."** - Gathering initial samples (first 1 second)
- **"Poor signal quality"** - Signal quality below 30%
- **"✓ Valid reading"** - Good quality reading available

## Usage Instructions

### 1. Upload Code
```bash
# Using Arduino IDE or PlatformIO
# Select board: ESP32 Dev Module
# Upload speed: 115200
```

### 2. Open Serial Monitor
- Baud rate: **115200**
- Line ending: Any

### 3. Place Finger on Sensor
- Place your **index finger** on the MAX30102 sensor
- Press **firmly** but not too hard
- Keep finger **still** during measurement
- Wait for "✓ Valid reading" status

### 4. Button Controls
- **Press once** to pause/resume logging
- **Press twice quickly** to reset readings
- **Hold for 2 seconds** to trigger SOS mode

## Troubleshooting

### No Sensor Detected
**Symptoms:** "Failed to initialize MAX30102!"

**Solutions:**
1. Check I2C connections (SDA=GPIO21, SCL=GPIO22)
2. Verify 3.3V power supply
3. Check I2C pull-up resistors (4.7kΩ recommended)
4. Test with I2C scanner sketch

### No Finger Detected
**Symptoms:** "Place finger on sensor" message

**Solutions:**
1. Press finger more firmly on sensor
2. Clean sensor surface
3. Try different finger
4. Check LED brightness (should see red glow)

### Poor Signal Quality
**Symptoms:** "Poor signal quality" or low quality percentage

**Solutions:**
1. Keep finger completely still
2. Adjust finger pressure (not too light, not too hard)
3. Ensure good contact with sensor
4. Avoid ambient light interference
5. Wait for sensor to warm up (30 seconds)

### Erratic Readings
**Symptoms:** Heart rate jumping around, unstable SpO2

**Solutions:**
1. Keep finger still during measurement
2. Ensure proper finger placement
3. Reset readings (double-press button)
4. Check for loose connections
5. Increase LED current if signal is weak

### Button Not Working
**Symptoms:** Button presses not detected

**Solutions:**
1. Check GPIO4 connection
2. Verify 3.3V power to button
3. Check button wiring (SIG to GPIO4, VCC to 3.3V, GND to GND)
4. Test with multimeter (should read HIGH when pressed)

## Algorithm Details

### Heart Rate Detection
1. Collect 100 samples in circular buffer
2. Detect peaks in IR signal (value higher than neighbors)
3. Calculate time interval between peaks
4. Convert to BPM: `BPM = 60000 / interval_ms`
5. Apply smoothing: `BPM_new = 0.7*BPM_old + 0.3*BPM_current`
6. Validate range (40-200 BPM)

### SpO2 Calculation
1. Calculate DC (average) for RED and IR signals
2. Calculate AC (max - min) for RED and IR signals
3. Compute R ratio: `R = (RED_AC/RED_DC) / (IR_AC/IR_DC)`
4. Apply formula: `SpO2 = 110 - 25*R`
5. Apply smoothing: `SpO2_new = 0.8*SpO2_old + 0.2*SpO2_current`
6. Clamp to range (70-100%)

## Performance Characteristics

- **Initialization Time:** ~1 second
- **Data Collection Time:** 1 second (100 samples)
- **Update Rate:** 1 reading per second
- **Memory Usage:** ~800 bytes (circular buffers)
- **CPU Usage:** Minimal (simple calculations)

## Calibration Notes

The SpO2 formula (`SpO2 = 110 - 25*R`) is a simplified empirical formula. For medical-grade accuracy:

1. **Calibrate against reference device** (pulse oximeter)
2. **Adjust formula coefficients** based on your sensor
3. **Consider individual variations** (skin tone, temperature)
4. **Test across range** (90-100% SpO2)

**Disclaimer:** This device is for educational purposes only and should not be used for medical diagnosis.

## Code Structure

```
sketch_nov8a/
├── sketch_nov8a.ino              # Main application
├── MAX30102_Driver.h/cpp         # Sensor driver
├── SOSButton_Driver.h/cpp        # Button driver
├── HeartRate_Service.h/cpp       # Heart rate & SpO2 service
├── HEART_RATE_MONITOR_GUIDE.md   # This guide
├── QUICK_START.md                # Quick reference
└── services/
    └── README.md                 # Service documentation
```

**Note:** All `.h` and `.cpp` files must be in the same directory as the `.ino` file for Arduino IDE to compile correctly.

## Future Enhancements

Potential improvements:
- [ ] Add OLED display for real-time visualization
- [ ] Implement data logging to SD card
- [ ] Add Bluetooth/WiFi data transmission
- [ ] Implement more sophisticated filtering (Butterworth, Kalman)
- [ ] Add heart rate variability (HRV) analysis
- [ ] Implement automatic gain control for LED current
- [ ] Add battery monitoring
- [ ] Create mobile app for data visualization

## References

- MAX30102 Datasheet: [Maxim Integrated](https://datasheets.maximintegrated.com/en/ds/MAX30102.pdf)
- SpO2 Calculation: [Pulse Oximetry Principles](https://en.wikipedia.org/wiki/Pulse_oximetry)
- ESP32 Documentation: [Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

