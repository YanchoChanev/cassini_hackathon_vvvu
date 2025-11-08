#include <SoftwareSerial.h>

#include "hr_module.h"
#include "gyro_module.h"
#include "ellipse_sim.h"

float bpm;
GyroReading g;
EllipseConfig cfg;
bool alert = false;

SoftwareSerial link(32, 33);

void setup() {
  Serial.begin(115200);
  
  link.begin(9600);

  HR_init(/*serialLogging=*/false, /*calibrationMode=*/false);

  Gyro_init(/*serialLogging=*/true);

  Ellipse_init(cfg);
}

void loop() {
  bpm = HR_step();
  Serial.print("BPM: ");
  Serial.println(bpm);

  if (Gyro_step(g)) {
    if (g.rollRate_dps > 100 || g.pitchRate_dps > 100) {
      alert = true;
    } else {
      alert = false;
    }

    Serial.print("roll=");  Serial.print(g.roll_deg, 1);
    Serial.print("  pitch="); Serial.print(g.pitch_deg, 1);
    Serial.print("  | rates dps: ");
    Serial.print(g.rollRate_dps, 1); Serial.print(", ");
    Serial.print(g.pitchRate_dps, 1); Serial.print(" | ");
    Serial.println(alert ? " !ALERT" : "");
  }

  EllipsePoint p;
  Ellipse_step(p);
  Serial.printf("lat=%.7f lon=%.7f\n", p.lat_deg, p.lon_deg);

  uint8_t pkt[4] = {0xAA, 0x01, 0x02, 0x55};
  link.write(pkt, sizeof(pkt));

  Serial.println();
  delay(200);
}
