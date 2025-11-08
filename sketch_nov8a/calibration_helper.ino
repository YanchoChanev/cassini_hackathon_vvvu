/*
 * MAX30102 Calibration Helper
 * 
 * This sketch helps you calibrate your MAX30102 sensor by:
 * 1. Displaying the R value (ratio of ratios)
 * 2. Showing calculated SpO2 with current formula
 * 3. Allowing you to compare with a reference pulse oximeter
 * 
 * Instructions:
 * 1. Upload this sketch
 * 2. Open Serial Monitor at 115200 baud
 * 3. Place finger on MAX30102 sensor
 * 4. Compare readings with medical-grade pulse oximeter
 * 5. Record R values and reference SpO2 values
 * 6. Use data to calculate calibration coefficients
 */

#include "MAX30102_Driver.h"
#include "HeartRate_Service.h"

// Create sensor instance
MAX30102_Driver heartSensor;

// Create heart rate service instance
HeartRate_Service hrService;

// Timing
unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 2000;  // Print every 2 seconds

// Calibration data collection
struct CalibrationPoint {
  float rValue;
  float calculatedSpO2;
  float referenceSpO2;  // You'll enter this manually
};

CalibrationPoint calibrationData[20];
int calibrationCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("\n\n========================================");
  Serial.println("MAX30102 CALIBRATION HELPER");
  Serial.println("========================================\n");

  // Initialize sensor
  Serial.println("Initializing MAX30102 sensor...");
  if (heartSensor.begin()) {
    Serial.println("✓ Sensor initialized!");
    
    Serial.print("Part ID: 0x");
    Serial.println(heartSensor.getPartID(), HEX);
    
    float temp = heartSensor.readTemperature();
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" °C\n");
  } else {
    Serial.println("✗ Sensor initialization failed!");
    Serial.println("Check connections and restart.");
    while (1) delay(1000);
  }

  // Initialize service
  hrService.begin();

  Serial.println("========================================");
  Serial.println("CALIBRATION INSTRUCTIONS:");
  Serial.println("========================================");
  Serial.println("1. Have a reference pulse oximeter ready");
  Serial.println("2. Place finger on MAX30102 sensor");
  Serial.println("3. Wait for stable readings");
  Serial.println("4. Compare with reference device");
  Serial.println("5. Record data in table below");
  Serial.println("6. Test in different conditions:");
  Serial.println("   - At rest (SpO2 ~98-100%)");
  Serial.println("   - After exercise (SpO2 ~95-98%)");
  Serial.println("   - Different people");
  Serial.println("========================================\n");

  Serial.println("CALIBRATION DATA:");
  Serial.println("Time | R Value | Calc SpO2 | Ref SpO2 | RED_AC | RED_DC | IR_AC | IR_DC | Quality");
  Serial.println("-----|---------|-----------|----------|--------|--------|-------|-------|--------");

  delay(2000);
}

void loop() {
  // Read sensor data
  if (heartSensor.available()) {
    MAX30102_Data data = heartSensor.readSample();
    
    if (data.valid) {
      hrService.addSample(data.red, data.ir);
    }
  }

  // Print calibration data
  unsigned long currentTime = millis();
  
  if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = currentTime;
    
    HeartRateData hrData = hrService.getReadings();
    
    if (hrData.fingerDetected && hrService.isReady()) {
      // Calculate R value (we need to expose this from the service)
      // For now, we'll recalculate it here
      float rValue = calculateRValue();
      
      // Format time
      unsigned long seconds = currentTime / 1000;
      
      // Print calibration row
      Serial.print(seconds);
      Serial.print("s | ");
      
      // R Value
      Serial.print(rValue, 4);
      Serial.print(" | ");
      
      // Calculated SpO2
      if (hrData.validReading) {
        Serial.print(hrData.spO2, 1);
        Serial.print("%");
      } else {
        Serial.print("---");
      }
      Serial.print(" | ");
      
      // Reference SpO2 (placeholder)
      Serial.print("____%");
      Serial.print(" | ");
      
      // Signal components (for debugging)
      printSignalComponents();
      
      // Signal quality
      Serial.print(hrData.signalQuality, 0);
      Serial.print("%");
      
      Serial.println();
      
      // Status message
      if (!hrData.validReading) {
        Serial.println("         ⚠️  Poor signal quality - adjust finger placement");
      } else if (hrData.signalQuality < 50) {
        Serial.println("         ⚠️  Low quality - keep finger still");
      } else {
        Serial.println("         ✓ Good reading - record reference SpO2 value");
      }
      
    } else if (!hrData.fingerDetected) {
      Serial.println("⚠️  No finger detected - place finger on sensor");
    } else {
      Serial.println("⏳ Collecting data...");
    }
  }
}

// Helper function to calculate R value
// This duplicates logic from HeartRate_Service but allows us to display it
float calculateRValue() {
  // We need access to the internal buffers
  // For a proper implementation, you'd add a getRValue() method to HeartRate_Service
  // For now, this is a placeholder
  return 0.0;  // You'll need to modify HeartRate_Service to expose this
}

// Helper function to print signal components
void printSignalComponents() {
  // This would need to be exposed from HeartRate_Service
  // For now, print placeholders
  Serial.print("--- | --- | --- | --- | ");
}

/*
 * CALIBRATION ANALYSIS
 * 
 * After collecting data, analyze it as follows:
 * 
 * 1. Create a table in Excel/Google Sheets:
 *    Column A: R Value
 *    Column B: Reference SpO2
 * 
 * 2. Calculate linear regression:
 *    In Excel: =SLOPE(B:B, A:A)  → This is your B coefficient
 *              =INTERCEPT(B:B, A:A)  → This is your A coefficient
 * 
 * 3. Update HeartRate_Service.cpp:
 *    Change: float spO2 = 110.0 - 25.0 * R;
 *    To:     float spO2 = A - B * R;
 *    
 *    Where A and B are your calculated coefficients
 * 
 * 4. Example:
 *    If you get A = 112.5 and B = 27.3, then:
 *    float spO2 = 112.5 - 27.3 * R;
 * 
 * 5. Validate:
 *    Test again with reference device
 *    Error should be < 2%
 */

