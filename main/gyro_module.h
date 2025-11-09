#pragma once
#include <Arduino.h>

struct GyroReading {
  float roll_deg;
  float pitch_deg;
  float rollRate_dps;
  float pitchRate_dps;
};

void Gyro_init(bool serialLogging = true, int sdaPin = 13, int sclPin = 14);

bool Gyro_step(GyroReading &out);
