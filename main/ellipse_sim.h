#pragma once
#include <Arduino.h>

struct EllipseConfig {
  double center_lat_deg = 42.6977;
  double center_lon_deg = 23.3219;
  double axis_x_m = 100.0;
  double axis_y_m = 70.0;
  double rotation_deg = 0.0;
  double start_phase_deg = 0.0;
  double period_sec = 240.0;
  double step_sec = 1.0;
  double noise_m = 0.8;
};

struct EllipsePoint {
  double lat_deg;
  double lon_deg;
  double east_m;
  double north_m;
  uint32_t step;
};

void Ellipse_init(const EllipseConfig& cfg);

bool Ellipse_step(EllipsePoint& out);

void Ellipse_reset(uint32_t step0 = 0);
