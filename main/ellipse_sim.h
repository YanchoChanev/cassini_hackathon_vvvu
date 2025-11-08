#pragma once
#include <Arduino.h>

struct EllipseConfig {
  double center_lat_deg = 42.6977;
  double center_lon_deg = 23.3219;
  double axis_x_m       = 100.0;   // semi-axis East  (m)
  double axis_y_m       = 70.0;    // semi-axis North (m)
  double rotation_deg   = 0.0;     // ellipse rotation in ground plane (deg)
  double start_phase_deg= 0.0;     // starting phase on the ellipse (deg)
  double period_sec     = 240.0;   // seconds per full lap
  double step_sec       = 1.0;     // seconds represented by each step() call
  double noise_m        = 0.8;     // random jitter radius (meters); 0 = none
};

struct EllipsePoint {
  double lat_deg;    // simulated latitude
  double lon_deg;    // simulated longitude
  double east_m;     // EN offsets with noise (for debugging/telemetry)
  double north_m;
  uint32_t step;     // step index
};

// Configure and reset internal state.
void Ellipse_init(const EllipseConfig& cfg);

// Advance one step and write results into 'out'. Returns true on success.
bool Ellipse_step(EllipsePoint& out);

// Reset the internal step counter (e.g., to jump phase).
void Ellipse_reset(uint32_t step0 = 0);
