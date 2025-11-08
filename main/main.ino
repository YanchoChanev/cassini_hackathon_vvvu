#include "hr_module.h"
#include "gyro_module.h"

float bpm;
GyroReading g;

void setup() {
  HR_init(/*serialLogging=*/false, /*calibrationMode=*/false);
  Gyro_init(/*serialLogging=*/true);
}

void loop() {
  bpm = HR_step();
  Serial.print("BPM: ");
  Serial.println(bpm);

  if (Gyro_step(g)) {
    Serial.print("roll=");  Serial.print(g.roll_deg, 1);
    Serial.print("  pitch="); Serial.print(g.pitch_deg, 1);
    Serial.print("  | rates dps: ");
    Serial.print(g.rollRate_dps, 1); Serial.print(", ");
    Serial.println(g.pitchRate_dps, 1);
  }

  delay(200);
}
