#include "hr_module.h"
#include <Arduino.h>

// ======= Instances (kept private to the module) =======
static MAX30102_Driver heartSensor;
static HeartRate_Service hrService;
static SOSButton_Driver sosButton(4, /*usePullUp=*/false, /*activeHigh=*/true);

// ======= Module state =======
static bool g_inited = false;
static bool g_logging = true;
static bool g_calibration = false;
static unsigned long g_lastPrint = 0;
static const unsigned long PRINT_INTERVAL = 1000; // ms
static unsigned long g_sampleCount = 0;

// ======= Button callback =======
static void onButtonEvent(ButtonState state) {
  if (!g_logging) {
    // still perform stateful actions even when quiet
  }

  // Toggle/Reset/SOS behavior stays internal; logging optional
  switch (state) {
    case BUTTON_PRESSED:
      // Toggle logging on short press
      g_logging = !g_logging;
      if (g_logging) {
        Serial.println(F("\n✅ LOGGING RESUMED\n"));
      } else {
        Serial.println(F("\n⛔ LOGGING PAUSED - Press button to resume\n"));
      }
      break;

    case BUTTON_RELEASED:
      if (g_logging) Serial.println(F("[BUTTON EVENT] Button RELEASED"));
      break;

    case BUTTON_LONG_PRESS:
      if (g_logging) {
        Serial.println(F("[BUTTON EVENT] ⚠️  LONG PRESS (2s) -> SOS MODE"));
        Serial.println(F("    >>> SOS MODE ACTIVATED (placeholder) <<<"));
      }
      break;

    case BUTTON_DOUBLE_PRESS:
      hrService.reset();
      g_sampleCount = 0;
      if (g_logging) {
        Serial.println(F("[BUTTON EVENT] ⚡ DOUBLE PRESS -> reset readings"));
        Serial.println(F("    Heart rate service reset complete!"));
      }
      break;

    default:
      if (g_logging) Serial.println(F("[BUTTON EVENT] Unknown"));
      break;
  }
}

// ======= Public API =======
void HR_init(bool serialLogging, bool calibrationMode) {
  if (g_inited) return;
  g_inited = true;
  g_logging = serialLogging;
  g_calibration = calibrationMode;

  if (g_logging) {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    Serial.println(F("\n\n================================="));
    Serial.println(F("Heart Rate & SpO2 Monitor"));
    Serial.println(F("MAX30102 + ESP32-WROOM-32D"));
    Serial.println(F("=================================\n"));
    Serial.println(F("Initializing SOS Button (GPIO4)..."));
  }

  // Button
  // sosButton.begin();
  // sosButton.setCallback(onButtonEvent);
  // if (g_logging) {
  //   Serial.println(F("✓ Button initialized."));
  //   Serial.print(F("Button initial state: "));
  //   Serial.println(sosButton.isPressed() ? F("PRESSED") : F("NOT PRESSED"));
  //   Serial.println();
  // }

  // Sensor
  if (g_logging) Serial.println(F("Initializing MAX30102 sensor..."));
  if (!heartSensor.begin()) {
    if (g_logging) {
      Serial.println(F("✗ Failed to initialize MAX30102!"));
      Serial.println(F("Check: SDA=GPIO21, SCL=GPIO22, 3.3V, pull-ups."));
    }
    // Stay initialized but non-functional; HR_step() will just return 0.
    return;
  }

  if (g_logging) {
    Serial.println(F("✓ MAX30102 initialized successfully!"));
    Serial.print(F("Part ID: 0x")); Serial.println(heartSensor.getPartID(), HEX);
    Serial.print(F("Revision ID: 0x")); Serial.println(heartSensor.getRevisionID(), HEX);

    float temp = heartSensor.readTemperature();
    Serial.print(F("Sensor Temperature: ")); Serial.print(temp); Serial.println(F(" °C"));

    Serial.println(F("\nSensor Configuration:"));
    Serial.println(F("- Mode: SpO2 (Red + IR)"));
    Serial.println(F("- Sample Rate: 100 Hz"));
    Serial.println(F("- Pulse Width: 411 μs"));
    Serial.println(F("- LED Current: 11 mA"));
    Serial.println(F("- FIFO Average: 4 samples"));

    Serial.println(F("\nInitializing Heart Rate Service..."));
  }

  hrService.begin();

  if (g_logging) {
    Serial.println(F("✓ Heart Rate Service initialized!"));
    Serial.println(F("\n================================="));
    Serial.println(F("Starting measurements... Place your finger on the sensor!"));
    Serial.println(F("=================================\n"));

    if (g_calibration) {
      Serial.println(F("⚙️  CALIBRATION MODE ENABLED"));
      Serial.println(F("Time | R Value | SpO2 | RED_AC | RED_DC | IR_AC  | IR_DC  | Quality | Status"));
      Serial.println(F("-----|---------|------|--------|--------|--------|--------|---------|--------\n"));
    } else {
      Serial.println(F("LOG FORMAT:"));
      Serial.println(F("Time | HR (bpm) | SpO2 (%) | Quality | Finger | Status"));
      Serial.println(F("-----+----------+----------+---------+--------+--------\n"));
    }
  }
}

// Returns bpm or 0.0 if not ready/invalid
float HR_step() {
  // Button state machine
  sosButton.update();

  // Edge-detect press to also allow pause/resume without callback latency
  static bool lastButtonState = false;
  bool curr = sosButton.isPressed();
  if (curr && !lastButtonState) {
    // short-press toggling handled in callback already
  }
  lastButtonState = curr;

  // Sensor sampling → service
  if (heartSensor.available()) {
    MAX30102_Data data = heartSensor.readSample();
    if (data.valid) {
      hrService.addSample(data.red, data.ir);
      g_sampleCount++;
    }
  }

  // Get computed readings
  HeartRateData hrData = hrService.getReadings();

  // Optional 1 Hz logging (does not affect return value)
  unsigned long now = millis();
  if (g_logging && (now - g_lastPrint >= PRINT_INTERVAL)) {
    g_lastPrint = now;

    unsigned long seconds = now / 1000;
    Serial.print(seconds); Serial.print(F("s | "));

    // HR
    if (hrData.validReading && hrData.heartRate > 0) {
      Serial.print(hrData.heartRate, 1); Serial.print(F(" bpm"));
    } else {
      Serial.print(F("---"));
    }
    Serial.print(F(" | "));

    // SpO2
    if (hrData.validReading && hrData.spO2 > 0) {
      Serial.print(hrData.spO2, 1); Serial.print(F("%"));
    } else {
      Serial.print(F("---%"));
    }
    Serial.print(F(" | "));

    // Quality, Finger, Status
    Serial.print(hrData.signalQuality, 0); Serial.print(F("% | "));
    Serial.print(hrData.fingerDetected ? F("YES") : F("NO ")); Serial.print(F(" | "));

    if (!hrData.fingerDetected)       Serial.println(F("Place finger on sensor"));
    else if (!hrService.isReady())    Serial.println(F("Collecting data..."));
    else if (!hrData.validReading)    Serial.println(F("Poor signal quality"));
    else                              Serial.println(F("✓ Valid reading"));
  }

  // Return only the heart-rate numeric value (or 0.0 if not available)
  if (hrData.validReading && hrData.heartRate > 0) {
    return (float)hrData.heartRate;
  }
  return 0.0f;
}
