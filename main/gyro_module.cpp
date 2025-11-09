#include "gyro_module.h"
#include <Wire.h>

static const uint8_t MPU_ADDR = 0x68;
static const float GYR_SENS = 65.5f;

static bool g_inited = false;
static bool g_log = true;
static float g_rateBiasRoll = 0.0f, g_rateBiasPitch = 0.0f, g_rateBiasYaw = 0.0f;
static float g_roll_deg = 0.0f, g_pitch_deg = 0.0f;
static uint32_t g_last_us = 0;

static void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

static bool mpuRead(uint8_t startReg, uint8_t *buf, size_t len) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(startReg);
  if (Wire.endTransmission(false) != 0) return false;
  int n = Wire.requestFrom((int)MPU_ADDR, (int)len);
  if (n != (int)len) return false;
  for (size_t i = 0; i < len; ++i) buf[i] = Wire.read();
  return true;
}

void Gyro_init(bool serialLogging, int sdaPin, int sclPin) {
  if (g_inited) return;
  g_inited = true;
  g_log = serialLogging;

  if (g_log) {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    Serial.println(F("\n[Gyro] Init..."));
  }

  mpuWrite(0x6B, 0x00);
  delay(100);

  mpuWrite(0x1A, 0x05);

  mpuWrite(0x1B, 0x08);

  const int N = 2000;
  float sumX = 0, sumY = 0, sumZ = 0;
  uint8_t raw[6];

  for (int i = 0; i < N; ++i) {
    if (mpuRead(0x43, raw, 6)) {
      int16_t gx = ((int16_t)raw[0] << 8) | raw[1];
      int16_t gy = ((int16_t)raw[2] << 8) | raw[3];
      int16_t gz = ((int16_t)raw[4] << 8) | raw[5];

      // Orientation convention: roll=X, pitch=-Y, yaw=-Z
      gy = -gy;
      gz = -gz;

      sumX += (gx / GYR_SENS);
      sumY += (gy / GYR_SENS);
      sumZ += (gz / GYR_SENS);
    }
    delay(1);
  }

  g_rateBiasRoll = sumX / N;
  g_rateBiasPitch = sumY / N;
  g_rateBiasYaw = sumZ / N;
  g_roll_deg = 0.0f;
  g_pitch_deg = 0.0f;
  g_last_us = 0;

  if (g_log) {
    Serial.println(F("[Gyro] Calibration complete."));
  }
}

bool Gyro_step(GyroReading &out) {
  if (!g_inited) return false;

  uint8_t raw[6];
  if (!mpuRead(0x43, raw, 6)) return false;

  int16_t gx = ((int16_t)raw[0] << 8) | raw[1];
  int16_t gy = ((int16_t)raw[2] << 8) | raw[3];
  int16_t gz = ((int16_t)raw[4] << 8) | raw[5];

  // Orientation convention: roll=X, pitch=-Y, yaw=-Z
  gy = -gy;
  gz = -gz;

  float rollRate_dps = (gx / GYR_SENS) - g_rateBiasRoll;
  float pitchRate_dps = (gy / GYR_SENS) - g_rateBiasPitch;

  uint32_t now = micros();
  if (g_last_us == 0) g_last_us = now;
  float dt = (now - g_last_us) * 1e-6f;
  g_last_us = now;
  if (dt <= 0.0f || dt > 0.1f) dt = 0.001f;

  g_roll_deg += rollRate_dps * dt;
  g_pitch_deg += pitchRate_dps * dt;

  out.roll_deg = g_roll_deg;
  out.pitch_deg = g_pitch_deg;
  out.rollRate_dps = rollRate_dps;
  out.pitchRate_dps = pitchRate_dps;

  return true;
}
