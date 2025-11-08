# Drivers Documentation

This directory contains hardware drivers for the ESP32-WROOM-32D project.

## Available Drivers

### 1. MAX30102_Driver
Driver for the MAX30102 heart rate and SpO2 sensor.

**Features:**
- I2C communication
- Configurable sample rates (50-3200 Hz)
- Configurable LED current
- FIFO buffer management
- Temperature reading
- Heart rate and SpO2 data acquisition

**Default Configuration:**
- I2C Pins: SDA=21, SCL=22 (ESP32 default)
- Sample Rate: 100 Hz
- Pulse Width: 411μs
- LED Current: 11mA (both Red and IR)
- FIFO Average: 4 samples

**Usage Example:**
```cpp
#include "drivers/MAX30102_Driver.h"

MAX30102_Driver heartSensor;

void setup() {
  if (heartSensor.begin()) {
    Serial.println("MAX30102 initialized!");
  }
}

void loop() {
  if (heartSensor.available()) {
    MAX30102_Data data = heartSensor.readSample();
    if (data.valid) {
      Serial.print("Red: ");
      Serial.print(data.red);
      Serial.print(" IR: ");
      Serial.println(data.ir);
    }
  }
}
```

### 2. SOSButton_Driver
Driver for a simple push button with advanced features.

**Features:**
- Debouncing
- Long press detection (default: 2 seconds)
- Double press detection (default: 400ms interval)
- Event callbacks
- Configurable active high/low
- Internal pull-up support

**Default Configuration:**
- Debounce Delay: 50ms
- Long Press Threshold: 2000ms (2 seconds)
- Double Press Interval: 400ms
- Pull-up: Enabled
- Active: Low (button connects to GND)

**Usage Example:**
```cpp
#include "drivers/SOSButton_Driver.h"

#define SOS_BUTTON_PIN 4

SOSButton_Driver sosButton(SOS_BUTTON_PIN);

void buttonCallback(ButtonState state) {
  switch(state) {
    case BUTTON_PRESSED:
      Serial.println("Button pressed!");
      break;
    case BUTTON_RELEASED:
      Serial.println("Button released!");
      break;
    case BUTTON_LONG_PRESS:
      Serial.println("SOS - Long press detected!");
      break;
    case BUTTON_DOUBLE_PRESS:
      Serial.println("Double press detected!");
      break;
  }
}

void setup() {
  sosButton.begin();
  sosButton.setCallback(buttonCallback);
}

void loop() {
  sosButton.update(); // Must be called regularly
}
```

## Pin Connections for ESP32-WROOM-32D

### MAX30102 Sensor
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21 (default, configurable)
- SCL → GPIO 22 (default, configurable)

### SOS Button
- One side → GPIO pin (e.g., GPIO 4)
- Other side → GND (when using internal pull-up)
- Optional: Add external 10kΩ pull-up resistor

## Notes
- The MAX30102 requires 3.3V power supply
- I2C pull-up resistors (4.7kΩ) may be needed if not present on sensor module
- The SOS button driver uses internal pull-up by default
- All drivers are non-blocking and suitable for use in the main loop

