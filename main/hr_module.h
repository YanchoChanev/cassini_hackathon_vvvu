#pragma once
#include "MAX30102_Driver.h"
#include "SOSButton_Driver.h"
#include "HeartRate_Service.h"

// Initialize the heart-rate subsystem (what used to live in setup()).
// - serialLogging: if true, the module prints status/log lines; if false, it stays quiet.
// - calibrationMode: if true, prints R/SpO2 calibration headers (only when logging enabled).
void HR_init(bool serialLogging = true, bool calibrationMode = false);

// Step the subsystem once (what used to live in loop()).
// - Handles button events, reads sensor, updates processing.
// - Returns current heart rate (bpm) if a valid reading is available, otherwise 0.0f.
// - Non-blocking; you choose your own pacing (e.g., call every 10–20 ms).
float HR_step();
