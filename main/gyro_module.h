#pragma once
#include <Arduino.h>

// Simple container for what you usually need
struct GyroReading {
  float roll_deg;       // integrated roll angle (deg)
  float pitch_deg;      // integrated pitch angle (deg)
  float rollRate_dps;   // instantaneous roll rate (deg/s)
  float pitchRate_dps;  // instantaneous pitch rate (deg/s)
};

// Initialize I2C + MPU6050 and run a short bias calibration.
// - If you use custom SDA/SCL on ESP32, pass them (defaults are Wire defaults).
// - serialLogging=true prints a few progress lines.
void Gyro_init(bool serialLogging = true, int sdaPin = 13, int sclPin = 14);

// Step the gyro once. Non-blocking, no delays.
// Returns true if a fresh sample was processed and 'out' was updated.
bool Gyro_step(GyroReading &out);
