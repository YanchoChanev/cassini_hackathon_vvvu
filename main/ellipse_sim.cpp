#include "ellipse_sim.h"
#include <math.h>

static EllipseConfig G;
static uint32_t g_step = 0;

static inline double deg2rad(double d) { return d * (PI / 180.0); }
static double urand() { return (double)random(-10000, 10001) / 10000.0; }

static void enMetersToLatLon(double east_m, double north_m,
                             double centerLatDeg, double centerLonDeg,
                             double& outLatDeg, double& outLonDeg)
{
  const double metersPerDegLat = 111320.0;
  const double metersPerDegLon = 111320.0 * cos(deg2rad(centerLatDeg));
  outLatDeg = centerLatDeg + (north_m / metersPerDegLat);
  outLonDeg = centerLonDeg + (east_m  / metersPerDegLon);
}

void Ellipse_init(const EllipseConfig& cfg) {
  G = cfg;
  g_step = 0;
  // seed RNG for jitter
  randomSeed((uint32_t)esp_timer_get_time());
}

bool Ellipse_step(EllipsePoint& out) {
  const double phi0 = deg2rad(G.start_phase_deg);
  const double psi  = deg2rad(G.rotation_deg);

  // radians advanced per call
  const double dphi = TWO_PI * (G.step_sec / G.period_sec);
  const double phi  = phi0 + g_step * dphi;

  // Unrotated ellipse in ENU
  const double x = G.axis_x_m * cos(phi); // east
  const double y = G.axis_y_m * sin(phi); // north

  // Rotate by psi
  const double xr = x * cos(psi) - y * sin(psi);
  const double yr = x * sin(psi) + y * cos(psi);

  // Optional jitter
  const double jx = (G.noise_m > 0.0) ? G.noise_m * urand() : 0.0;
  const double jy = (G.noise_m > 0.0) ? G.noise_m * urand() : 0.0;

  const double east_m  = xr + jx;
  const double north_m = yr + jy;

  double latDeg, lonDeg;
  enMetersToLatLon(east_m, north_m, G.center_lat_deg, G.center_lon_deg, latDeg, lonDeg);

  out.lat_deg  = latDeg;
  out.lon_deg  = lonDeg;
  out.east_m   = east_m;
  out.north_m  = north_m;
  out.step     = g_step;

  ++g_step;
  return true;
}

void Ellipse_reset(uint32_t step0) {
  g_step = step0;
}
