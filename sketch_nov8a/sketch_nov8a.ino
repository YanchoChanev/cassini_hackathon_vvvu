#include "MAX30102_Driver.h"
#include "SOSButton_Driver.h"
#include "HeartRate_Service.h"

// Create sensor instance
MAX30102_Driver heartSensor;

// Create heart rate service instance
HeartRate_Service hrService;

// Create button instance for Grove Button (GPIO4, with internal pull-down, active HIGH)
// Grove Button is active HIGH - button pressed = HIGH signal
SOSButton_Driver sosButton(4, false, true);

// Timing variables
unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 1000;   // Print every 1 second for heart rate

// Control flag for sensor logging
bool sensorLoggingEnabled = true;

// Sample counter
unsigned long sampleCount = 0;

// Calibration mode (set to true to see R values for calibration)
bool calibrationMode = false;  // Change to true to enable calibration output

// Button event callback function
void onButtonEvent(ButtonState state) {
  Serial.print("\n[BUTTON EVENT] ");

  switch (state) {
    case BUTTON_PRESSED:
      Serial.println("Button PRESSED");
      break;

    case BUTTON_RELEASED:
      Serial.println("Button RELEASED");
      break;

    case BUTTON_LONG_PRESS:
      Serial.println("⚠️  LONG PRESS detected (2 seconds)!");
      Serial.println("    >>> SOS MODE ACTIVATED <<<");
      Serial.println("    Emergency signal would be sent!");
      break;

    case BUTTON_DOUBLE_PRESS:
      Serial.println("⚡ DOUBLE PRESS detected!");
      Serial.println("    >>> Resetting heart rate readings <<<");
      hrService.reset();
      sampleCount = 0;
      Serial.println("    Heart rate service reset complete!");
      break;

    default:
      Serial.println("Unknown event");
      break;
  }
  Serial.println();
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("\n\n=================================");
  Serial.println("Heart Rate & SpO2 Monitor");
  Serial.println("MAX30102 + ESP32-WROOM-32D");
  Serial.println("=================================\n");

  // Initialize the button
  Serial.println("Initializing SOS Button (GPIO4)...");
  sosButton.begin();
  sosButton.setCallback(onButtonEvent);
  Serial.println("✓ Button initialized successfully!");
  Serial.println("  - Single press: Toggle logging");
  Serial.println("  - Long press (2s): SOS trigger");
  Serial.println("  - Double press: Reset readings");

  // Test button immediately
  Serial.print("\nButton initial state: ");
  Serial.println(sosButton.isPressed() ? "PRESSED" : "NOT PRESSED");
  Serial.println();

  // Initialize the MAX30102 sensor
  Serial.println("Initializing MAX30102 sensor...");

  if (heartSensor.begin()) {
    Serial.println("✓ MAX30102 initialized successfully!");

    // Print sensor information
    Serial.print("Part ID: 0x");
    Serial.println(heartSensor.getPartID(), HEX);
    Serial.print("Revision ID: 0x");
    Serial.println(heartSensor.getRevisionID(), HEX);

    // Read temperature
    float temp = heartSensor.readTemperature();
    Serial.print("Sensor Temperature: ");
    Serial.print(temp);
    Serial.println(" °C");

    Serial.println("\nSensor Configuration:");
    Serial.println("- Mode: SpO2 (Red + IR)");
    Serial.println("- Sample Rate: 100 Hz");
    Serial.println("- Pulse Width: 411 μs");
    Serial.println("- LED Current: 11 mA");
    Serial.println("- FIFO Average: 4 samples");

    // Initialize heart rate service
    Serial.println("\nInitializing Heart Rate Service...");
    hrService.begin();
    Serial.println("✓ Heart Rate Service initialized!");

    Serial.println("\n=================================");
    Serial.println("Starting measurements...");
    Serial.println("Place your finger on the sensor!");
    Serial.println("=================================\n");

    if (calibrationMode) {
      Serial.println("⚙️  CALIBRATION MODE ENABLED");
      Serial.println("Compare R values with reference pulse oximeter\n");
      Serial.println("CALIBRATION LOG:");
      Serial.println("Time | R Value | SpO2 | RED_AC | RED_DC | IR_AC  | IR_DC  | Quality | Status");
      Serial.println("-----|---------|------|--------|--------|--------|--------|---------|--------\n");
    } else {
      Serial.println("LOG FORMAT:");
      Serial.println("Time | HR (bpm) | SpO2 (%) | Quality | Finger | Status");
      Serial.println("-----+----------+----------+---------+--------+--------\n");
    }

    delay(1000);
  } else {
    Serial.println("✗ Failed to initialize MAX30102!");
    Serial.println("Please check:");
    Serial.println("  - Sensor connections (SDA=GPIO21, SCL=GPIO22)");
    Serial.println("  - Power supply (3.3V)");
    Serial.println("  - I2C pull-up resistors");
    while (1) {
      delay(1000);
    }
  }
}

void loop() {
  // Update button state (MUST be called frequently!)
  sosButton.update();

  // Check for button push detection
  static bool lastButtonState = false;
  bool currentButtonState = sosButton.isPressed();

  // Detect button push (transition from not pressed to pressed)
  if (currentButtonState && !lastButtonState) {
    // Toggle sensor logging when button is pressed
    if (sensorLoggingEnabled) {
      sensorLoggingEnabled = false;
      Serial.println("\n⛔ LOGGING PAUSED - Press button to resume\n");
    } else {
      sensorLoggingEnabled = true;
      Serial.println("\n✅ LOGGING RESUMED\n");
    }
  }

  // Update last state
  lastButtonState = currentButtonState;

  // Read sensor data and feed to heart rate service
  if (heartSensor.available()) {
    MAX30102_Data data = heartSensor.readSample();

    if (data.valid) {
      // Add sample to heart rate service
      hrService.addSample(data.red, data.ir);
      sampleCount++;
    }
  }

  // Print heart rate and SpO2 readings every second (only if logging is enabled)
  unsigned long currentTime = millis();

  if (sensorLoggingEnabled && (currentTime - lastPrintTime >= PRINT_INTERVAL)) {
    lastPrintTime = currentTime;

    // Get readings from heart rate service
    HeartRateData hrData = hrService.getReadings();

    // Format time (seconds)
    unsigned long seconds = currentTime / 1000;

    // Print in log format
    Serial.print(seconds);
    Serial.print("s | ");

    // Heart Rate
    if (hrData.validReading && hrData.heartRate > 0) {
      Serial.print(hrData.heartRate, 1);
      Serial.print(" bpm");
    } else {
      Serial.print("---");
    }
    Serial.print(" | ");

    // SpO2
    if (hrData.validReading && hrData.spO2 > 0) {
      Serial.print(hrData.spO2, 1);
      Serial.print("%");
    } else {
      Serial.print("---%");
    }
    Serial.print(" | ");

    // Signal Quality
    Serial.print(hrData.signalQuality, 0);
    Serial.print("%");
    Serial.print(" | ");

    // Finger Detection
    Serial.print(hrData.fingerDetected ? "YES" : "NO ");
    Serial.print(" | ");

    // Status
    if (!hrData.fingerDetected) {
      Serial.println("Place finger on sensor");
    } else if (!hrService.isReady()) {
      Serial.println("Collecting data...");
    } else if (!hrData.validReading) {
      Serial.println("Poor signal quality");
    } else {
      Serial.println("✓ Valid reading");
    }
  }

  delay(10); // Small delay to prevent overwhelming the serial
}
