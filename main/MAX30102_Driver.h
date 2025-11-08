#ifndef MAX30102_DRIVER_H
#define MAX30102_DRIVER_H

#include <Arduino.h>
#include <Wire.h>

// MAX30102 I2C Address
#define MAX30102_ADDRESS 0x57

// MAX30102 Registers
#define MAX30102_INT_STATUS_1    0x00
#define MAX30102_INT_STATUS_2    0x01
#define MAX30102_INT_ENABLE_1    0x02
#define MAX30102_INT_ENABLE_2    0x03
#define MAX30102_FIFO_WR_PTR     0x04
#define MAX30102_FIFO_OVF_CNT    0x05
#define MAX30102_FIFO_RD_PTR     0x06
#define MAX30102_FIFO_DATA       0x07
#define MAX30102_FIFO_CONFIG     0x08
#define MAX30102_MODE_CONFIG     0x09
#define MAX30102_SPO2_CONFIG     0x0A
#define MAX30102_LED1_PA         0x0C
#define MAX30102_LED2_PA         0x0D
#define MAX30102_PILOT_PA        0x10
#define MAX30102_MULTI_LED_CTRL1 0x11
#define MAX30102_MULTI_LED_CTRL2 0x12
#define MAX30102_TEMP_INT        0x1F
#define MAX30102_TEMP_FRAC       0x20
#define MAX30102_TEMP_CONFIG     0x21
#define MAX30102_REV_ID          0xFE
#define MAX30102_PART_ID         0xFF

// Mode Configuration
#define MAX30102_MODE_HR_ONLY    0x02
#define MAX30102_MODE_SPO2       0x03
#define MAX30102_MODE_MULTI_LED  0x07

// Sample Rate
#define MAX30102_SAMPLE_RATE_50   0x00
#define MAX30102_SAMPLE_RATE_100  0x01
#define MAX30102_SAMPLE_RATE_200  0x02
#define MAX30102_SAMPLE_RATE_400  0x03
#define MAX30102_SAMPLE_RATE_800  0x04
#define MAX30102_SAMPLE_RATE_1000 0x05
#define MAX30102_SAMPLE_RATE_1600 0x06
#define MAX30102_SAMPLE_RATE_3200 0x07

// Pulse Width
#define MAX30102_PULSE_WIDTH_69   0x00
#define MAX30102_PULSE_WIDTH_118  0x01
#define MAX30102_PULSE_WIDTH_215  0x02
#define MAX30102_PULSE_WIDTH_411  0x03

// LED Current (mA)
#define MAX30102_LED_CURRENT_0MA    0x00
#define MAX30102_LED_CURRENT_4_4MA  0x0F
#define MAX30102_LED_CURRENT_7_6MA  0x1F
#define MAX30102_LED_CURRENT_11MA   0x2F
#define MAX30102_LED_CURRENT_14_2MA 0x3F
#define MAX30102_LED_CURRENT_17_4MA 0x4F
#define MAX30102_LED_CURRENT_20_8MA 0x5F
#define MAX30102_LED_CURRENT_24MA   0x6F
#define MAX30102_LED_CURRENT_27_1MA 0x7F
#define MAX30102_LED_CURRENT_30_6MA 0x8F
#define MAX30102_LED_CURRENT_33_8MA 0x9F
#define MAX30102_LED_CURRENT_37MA   0xAF
#define MAX30102_LED_CURRENT_40_2MA 0xBF
#define MAX30102_LED_CURRENT_43_6MA 0xCF
#define MAX30102_LED_CURRENT_46_8MA 0xDF
#define MAX30102_LED_CURRENT_50MA   0xFF

struct MAX30102_Data {
  uint32_t red;
  uint32_t ir;
  bool valid;
};

class MAX30102_Driver {
private:
  uint8_t _sdaPin;
  uint8_t _sclPin;
  bool _initialized;
  
  // Helper functions
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  void bitMask(uint8_t reg, uint8_t mask, uint8_t value);
  
public:
  MAX30102_Driver(uint8_t sdaPin = 26, uint8_t sclPin = 27);
  
  // Initialization and configuration
  bool begin();
  void reset();
  void shutdown();
  void wakeup();
  
  // Configuration methods
  void setMode(uint8_t mode);
  void setSampleRate(uint8_t sampleRate);
  void setPulseWidth(uint8_t pulseWidth);
  void setLEDCurrent(uint8_t redLED, uint8_t irLED);
  void setFIFOAverage(uint8_t samples);
  void enableFIFORollover(bool enable);
  
  // Data reading
  bool available();
  MAX30102_Data readSample();
  void clearFIFO();
  
  // Temperature reading
  float readTemperature();
  
  // Diagnostic
  uint8_t getPartID();
  uint8_t getRevisionID();
  bool checkConnection();
};

#endif // MAX30102_DRIVER_H

