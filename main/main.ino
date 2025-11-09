#include "hr_module.h"
#include "gyro_module.h"
#include "ellipse_sim.h"

float bpm;
GyroReading g;
EllipseConfig cfg;
bool alert = false;

HardwareSerial Link(2);

void setup() {
  Serial.begin(115200);
  Link.begin(9600, SERIAL_8N1, 32, 33);

  HR_init(/*serialLogging=*/false, /*calibrationMode=*/false);

  Gyro_init(/*serialLogging=*/true);

  Ellipse_init(cfg);
}

void loop() {
  // HR sensor
  bpm = HR_step();
  Serial.print("BPM: ");
  Serial.println(bpm);

  // Gyro sensor
  if (Gyro_step(g)) {
    if (g.rollRate_dps > 100 || g.pitchRate_dps > 100) {
      alert = true;
    } else {
      alert = false;
    }
    Serial.print("roll=");
    Serial.print(g.roll_deg, 1);
    Serial.print("  pitch=");
    Serial.print(g.pitch_deg, 1);
    Serial.print("  | rates dps: ");
    Serial.print(g.rollRate_dps, 1);
    Serial.print(", ");
    Serial.print(g.pitchRate_dps, 1);
    Serial.print(" | ");
    Serial.println(alert ? " !ALERT" : "");
  }

  // GPS position simulator
  EllipsePoint p;
  Ellipse_step(p);
  Serial.printf("lat=%.7f lon=%.7f\n", p.lat_deg, p.lon_deg);

  // Send payload to satellite
  const float lat = p.lat_deg;
  const float lon = p.lon_deg;
  const uint8_t hr = bpm;
  const int32_t id = 1234;
  uint8_t payload[14];
  size_t off = 0;
  payload[off++] = (uint8_t)(alert ? 'a' : 'd');
  memcpy(payload + off, &lat, sizeof(lat));
  off += sizeof(lat);
  memcpy(payload + off, &lon, sizeof(lon));
  off += sizeof(lon);
  payload[off++] = hr;
  memcpy(payload + off, &id, sizeof(id));
  off += sizeof(id);
  Link.write(payload, sizeof(payload));

  // Pacing
  Serial.println();
  delay(200);
}
