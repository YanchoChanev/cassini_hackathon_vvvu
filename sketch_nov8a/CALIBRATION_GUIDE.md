# MAX30102 Calibration Guide

## Overview

The MAX30102 sensor requires calibration for accurate SpO2 readings. Heart rate is generally accurate without calibration.

## What is the R Value?

The R value is the ratio of ratios:
```
R = (RED_AC / RED_DC) / (IR_AC / IR_DC)

Where:
- AC = Signal variation (max - min)
- DC = Signal average
- RED = Red LED (660nm)
- IR = Infrared LED (880nm)
```

## Current Formula

```cpp
SpO2 = 110.0 - 25.0 * R
```

This is a **simplified empirical formula** that works reasonably well but may need adjustment.

## Calibration Methods

### Method 1: Simple Two-Point Calibration

**Equipment Needed:**
- Medical-grade pulse oximeter (reference device)
- Your MAX30102 sensor

**Steps:**

1. **Add Debug Output to Code**

Add this to your `HeartRate_Service.cpp` in the `calculateSpO2()` function:

```cpp
float HeartRate_Service::calculateSpO2() {
  // ... existing code ...
  
  // Calculate R value (ratio of ratios)
  float R = (_redAC / _redDC) / (_irAC / _irDC);
  
  // DEBUG: Print R value for calibration
  Serial.print("[CALIBRATION] R = ");
  Serial.print(R, 4);
  Serial.print(" | ");
  
  // SpO2 calculation using empirical formula
  float spO2 = 110.0 - 25.0 * R;
  
  // ... rest of code ...
}
```

2. **Collect Reference Data**

Measure simultaneously with both devices:

| Condition | R Value | MAX30102 SpO2 | Reference SpO2 |
|-----------|---------|---------------|----------------|
| Resting   | 0.45    | 98.7%         | 99%            |
| Normal    | 0.50    | 97.5%         | 98%            |

3. **Calculate Correction**

If your readings are consistently off by a fixed amount:
```cpp
// Simple offset correction
float spO2 = (110.0 - 25.0 * R) + OFFSET;

// Example: If readings are 2% low
const float SPO2_OFFSET = 2.0;
float spO2 = (110.0 - 25.0 * R) + SPO2_OFFSET;
```

If the error varies with SpO2 level, adjust the coefficients:
```cpp
// Adjust slope and intercept
float spO2 = A - B * R;

// Example calibrated values:
const float SPO2_A = 112.0;  // Instead of 110.0
const float SPO2_B = 26.5;   // Instead of 25.0
float spO2 = SPO2_A - SPO2_B * R;
```

### Method 2: Multi-Point Calibration (Advanced)

**Equipment Needed:**
- Medical-grade pulse oximeter
- Multiple test subjects (different skin tones)
- Spreadsheet software (Excel, Google Sheets)

**Steps:**

1. **Collect Multiple Data Points**

Test on 5-10 different people in different conditions:

```
Person | Condition      | R Value | Reference SpO2
-------|----------------|---------|---------------
1      | Resting        | 0.42    | 99%
1      | After exercise | 0.55    | 96%
2      | Resting        | 0.45    | 98%
2      | After exercise | 0.58    | 95%
3      | Resting        | 0.48    | 99%
...
```

2. **Linear Regression**

Use spreadsheet to calculate best-fit line:
- X-axis: R values
- Y-axis: Reference SpO2 values
- Calculate: `SpO2 = A - B * R`

In Excel/Google Sheets:
```
=SLOPE(SpO2_range, R_range)  → gives you B
=INTERCEPT(SpO2_range, R_range)  → gives you A
```

3. **Update Code**

```cpp
// Replace in HeartRate_Service.cpp
const float SPO2_CALIBRATION_A = 112.5;  // Your calculated value
const float SPO2_CALIBRATION_B = 27.3;   // Your calculated value

float spO2 = SPO2_CALIBRATION_A - SPO2_CALIBRATION_B * R;
```

### Method 3: Manufacturer's Calibration Curve

Some manufacturers provide calibration curves. Maxim Integrated suggests:

```cpp
// Polynomial calibration (more accurate)
float spO2 = -45.060 * R * R + 30.354 * R + 94.845;
```

Or the simpler linear version:
```cpp
float spO2 = 110.0 - 25.0 * R;
```

## Factors Affecting Accuracy

### 1. **LED Current**
Higher current = stronger signal but more power consumption

Current setting in `MAX30102_Driver.cpp`:
```cpp
// Try different values: 3.1mA, 6.2mA, 11mA, 25.4mA, 50mA
setLEDCurrent(0x1F, 0x1F);  // 11mA for both LEDs
```

**Calibration tip:** If signal is weak (low AC values), increase LED current.

### 2. **Skin Tone**
Darker skin absorbs more light, affecting R value.

**Solution:** Calibrate with multiple skin tones or use adaptive calibration.

### 3. **Finger Pressure**
Too much pressure restricts blood flow.

**Solution:** Instruct users to apply gentle, consistent pressure.

### 4. **Ambient Light**
Bright light can interfere with readings.

**Solution:** Shield sensor or use in normal indoor lighting.

### 5. **Temperature**
Sensor performance varies with temperature.

**Solution:** Allow 30-second warmup period.

## Recommended Calibration Procedure

### Quick Calibration (10 minutes)

1. **Get a reference pulse oximeter**
2. **Measure yourself at rest** (should be 95-100%)
3. **Record R value and both SpO2 readings**
4. **Do light exercise** (jumping jacks for 30 seconds)
5. **Measure again** (should be 92-98%)
6. **Calculate offset or adjust coefficients**

### Example Calculation

**Data collected:**
```
Condition | R Value | MAX30102 | Reference | Error
----------|---------|----------|-----------|------
Resting   | 0.45    | 98.7%    | 99.0%     | -0.3%
Exercise  | 0.60    | 95.0%    | 96.5%     | -1.5%
```

**Analysis:**
- Average error: -0.9%
- Error increases at lower SpO2

**Solution 1 - Simple Offset:**
```cpp
float spO2 = (110.0 - 25.0 * R) + 1.0;  // Add 1% offset
```

**Solution 2 - Adjust Coefficients:**
```cpp
// Increase intercept slightly
float spO2 = 111.0 - 25.0 * R;
```

## Code Implementation

### Option 1: Add Calibration Constants

Add to `HeartRate_Service.h`:
```cpp
// Calibration constants (adjust based on your testing)
#define SPO2_CALIBRATION_A 110.0  // Intercept
#define SPO2_CALIBRATION_B 25.0   // Slope
#define SPO2_CALIBRATION_OFFSET 0.0  // Fixed offset
```

Update `HeartRate_Service.cpp`:
```cpp
float HeartRate_Service::calculateSpO2() {
  // ... existing code to calculate R ...
  
  // Apply calibrated formula
  float spO2 = SPO2_CALIBRATION_A - SPO2_CALIBRATION_B * R + SPO2_CALIBRATION_OFFSET;
  
  // ... rest of code ...
}
```

### Option 2: Runtime Calibration

Add calibration mode to your sketch:
```cpp
bool calibrationMode = false;

void loop() {
  // ... existing code ...
  
  if (calibrationMode) {
    // Print R value for calibration
    HeartRateData hrData = hrService.getReadings();
    if (hrData.validReading) {
      float R = calculateRValue();  // You'd need to expose this
      Serial.print("R: ");
      Serial.print(R, 4);
      Serial.print(" | SpO2: ");
      Serial.print(hrData.spO2, 1);
      Serial.println(" | Enter reference SpO2:");
    }
  }
}
```

## Validation

After calibration, validate your results:

1. **Test on multiple people** (at least 3-5)
2. **Compare with reference device**
3. **Acceptable error:** ±2% for SpO2
4. **If error > 2%:** Recalibrate or check hardware

## Typical R Values

For reference:
```
SpO2 Level | Typical R Value
-----------|----------------
100%       | 0.40 - 0.45
98%        | 0.45 - 0.50
95%        | 0.50 - 0.60
90%        | 0.60 - 0.75
85%        | 0.75 - 0.90
```

## Troubleshooting

### Problem: R values outside typical range

**Causes:**
- LED current too low/high
- Poor finger contact
- Sensor malfunction

**Solutions:**
- Adjust LED current
- Check finger placement
- Verify sensor connections

### Problem: Inconsistent readings

**Causes:**
- Movement
- Ambient light
- Poor signal quality

**Solutions:**
- Keep finger still
- Shield from bright light
- Check signal quality metric

## Advanced: Polynomial Calibration

For better accuracy across full range:

```cpp
// Quadratic formula
float spO2 = A * R * R + B * R + C;

// Example values (you'd calculate from your data)
const float A = -45.060;
const float B = 30.354;
const float C = 94.845;

float spO2 = A * R * R + B * R + C;
```

## Medical Disclaimer

⚠️ **Important:**
- This device is for **educational purposes only**
- **Not FDA approved** for medical use
- **Do not use** for medical diagnosis
- Always consult healthcare professionals for medical concerns
- Calibration does not make this a medical device

## Summary

**Minimum Calibration:**
1. Compare with reference device
2. Adjust offset: `spO2 = (110.0 - 25.0 * R) + OFFSET`

**Better Calibration:**
1. Collect 5-10 data points
2. Calculate new A and B coefficients
3. Update: `spO2 = A - B * R`

**Best Calibration:**
1. Test multiple subjects and conditions
2. Use polynomial regression
3. Validate thoroughly

