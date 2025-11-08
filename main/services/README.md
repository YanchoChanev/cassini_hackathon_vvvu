# Services

This directory contains service-layer components that implement business logic and coordinate between drivers.

## Overview

Services provide higher-level functionality by:
- Coordinating multiple drivers
- Implementing application logic
- Managing state and data processing
- Providing clean interfaces for the main application

## Available Services

### 1. HeartRate_Service
Service for calculating heart rate (BPM) and blood oxygen saturation (SpO2) from MAX30102 sensor data.

**Features:**
- Real-time heart rate calculation using peak detection
- SpO2 (blood oxygen saturation) calculation
- Finger detection
- Signal quality assessment
- Automatic data smoothing and filtering
- Circular buffer for efficient data processing

**Configuration:**
- Buffer Size: 100 samples
- Sample Rate: 100 Hz (must match MAX30102 configuration)
- Valid Heart Rate Range: 40-200 BPM
- Valid SpO2 Range: 70-100%
- Finger Detection Threshold: IR > 50,000

**Usage Example:**
```cpp
#include "services/HeartRate_Service.h"
#include "MAX30102_Driver.h"

MAX30102_Driver sensor;
HeartRate_Service hrService;

void setup() {
  sensor.begin();
  hrService.begin();
}

void loop() {
  if (sensor.available()) {
    MAX30102_Data data = sensor.readSample();
    
    if (data.valid) {
      // Add sample to service
      hrService.addSample(data.red, data.ir);
      
      // Get calculated readings
      HeartRateData hrData = hrService.getReadings();
      
      if (hrData.validReading) {
        Serial.print("Heart Rate: ");
        Serial.print(hrData.heartRate);
        Serial.println(" BPM");
        
        Serial.print("SpO2: ");
        Serial.print(hrData.spO2);
        Serial.println(" %");
        
        Serial.print("Signal Quality: ");
        Serial.print(hrData.signalQuality);
        Serial.println(" %");
      }
    }
  }
}
```

**Data Structure:**
```cpp
struct HeartRateData {
  float heartRate;        // Heart rate in BPM
  float spO2;             // Blood oxygen saturation percentage
  bool fingerDetected;    // Is finger on sensor?
  bool validReading;      // Is the reading valid?
  uint32_t lastBeatTime;  // Timestamp of last detected beat
  float signalQuality;    // Signal quality indicator (0-100%)
};
```

**Methods:**
- `begin()` - Initialize the service
- `addSample(red, ir)` - Add new RED and IR sample for processing
- `getReadings()` - Get current heart rate and SpO2 readings
- `reset()` - Clear all buffers and reset state
- `isReady()` - Check if enough data has been collected

**Algorithm Details:**

1. **Heart Rate Detection:**
   - Uses IR signal for peak detection (more stable than RED)
   - Detects peaks when value is higher than neighbors and above threshold
   - Calculates BPM from time interval between peaks
   - Applies exponential moving average for smoothing

2. **SpO2 Calculation:**
   - Calculates AC (variation) and DC (average) for both RED and IR
   - Computes R value: (RED_AC/RED_DC) / (IR_AC/IR_DC)
   - Uses empirical formula: SpO2 = 110 - 25*R
   - Applies smoothing and range clamping

3. **Signal Quality:**
   - Based on signal strength (IR DC value)
   - Signal variation (IR AC value)
   - Signal stability (standard deviation)
   - Combined weighted score (0-100%)

**Notes:**
- Requires at least 100 samples (1 second at 100Hz) before providing readings
- SpO2 formula is simplified and may need calibration for accuracy
- Best results when finger is placed firmly on sensor
- Avoid movement during measurement

