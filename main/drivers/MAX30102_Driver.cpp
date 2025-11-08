#include "MAX30102_Driver.h"

MAX30102_Driver::MAX30102_Driver(uint8_t sdaPin, uint8_t sclPin) {
  _sdaPin = sdaPin;
  _sclPin = sclPin;
  _initialized = false;
}

bool MAX30102_Driver::begin() {
  // Initialize I2C
  Wire.begin(_sdaPin, _sclPin);
  Wire.setClock(400000); // 400kHz I2C
  
  delay(100); // Wait for sensor to stabilize
  
  // Check if sensor is connected
  if (!checkConnection()) {
    return false;
  }
  
  // Reset the sensor
  reset();
  delay(100);
  
  // Default configuration for SpO2 mode
  setFIFOAverage(4);           // Average 4 samples
  enableFIFORollover(true);    // Enable FIFO rollover
  setMode(MAX30102_MODE_SPO2); // SpO2 mode (Red + IR)
  setSampleRate(MAX30102_SAMPLE_RATE_100); // 100 samples per second
  setPulseWidth(MAX30102_PULSE_WIDTH_411); // 411us pulse width
  setLEDCurrent(MAX30102_LED_CURRENT_11MA, MAX30102_LED_CURRENT_11MA); // 11mA for both LEDs
  
  clearFIFO();
  
  _initialized = true;
  return true;
}

void MAX30102_Driver::reset() {
  writeRegister(MAX30102_MODE_CONFIG, 0x40);
  delay(100);
}

void MAX30102_Driver::shutdown() {
  bitMask(MAX30102_MODE_CONFIG, 0x7F, 0x80);
}

void MAX30102_Driver::wakeup() {
  bitMask(MAX30102_MODE_CONFIG, 0x7F, 0x00);
}

void MAX30102_Driver::setMode(uint8_t mode) {
  bitMask(MAX30102_MODE_CONFIG, 0xF8, mode);
}

void MAX30102_Driver::setSampleRate(uint8_t sampleRate) {
  bitMask(MAX30102_SPO2_CONFIG, 0xE3, sampleRate << 2);
}

void MAX30102_Driver::setPulseWidth(uint8_t pulseWidth) {
  bitMask(MAX30102_SPO2_CONFIG, 0xFC, pulseWidth);
}

void MAX30102_Driver::setLEDCurrent(uint8_t redLED, uint8_t irLED) {
  writeRegister(MAX30102_LED1_PA, redLED);
  writeRegister(MAX30102_LED2_PA, irLED);
}

void MAX30102_Driver::setFIFOAverage(uint8_t samples) {
  uint8_t avgValue = 0;
  if (samples == 1) avgValue = 0;
  else if (samples == 2) avgValue = 1;
  else if (samples == 4) avgValue = 2;
  else if (samples == 8) avgValue = 3;
  else if (samples == 16) avgValue = 4;
  else if (samples == 32) avgValue = 5;
  else avgValue = 2; // Default to 4 samples
  
  bitMask(MAX30102_FIFO_CONFIG, 0x1F, avgValue << 5);
}

void MAX30102_Driver::enableFIFORollover(bool enable) {
  bitMask(MAX30102_FIFO_CONFIG, 0xEF, enable ? 0x10 : 0x00);
}

bool MAX30102_Driver::available() {
  uint8_t writePtr = readRegister(MAX30102_FIFO_WR_PTR);
  uint8_t readPtr = readRegister(MAX30102_FIFO_RD_PTR);
  
  return (writePtr != readPtr);
}

MAX30102_Data MAX30102_Driver::readSample() {
  MAX30102_Data data;
  data.valid = false;
  
  if (!available()) {
    return data;
  }
  
  // Read 6 bytes from FIFO (3 bytes Red + 3 bytes IR)
  Wire.beginTransmission(MAX30102_ADDRESS);
  Wire.write(MAX30102_FIFO_DATA);
  Wire.endTransmission(false);
  
  Wire.requestFrom(MAX30102_ADDRESS, 6);
  
  if (Wire.available() >= 6) {
    // Read Red LED data (18-bit)
    uint32_t red = 0;
    red |= (uint32_t)Wire.read() << 16;
    red |= (uint32_t)Wire.read() << 8;
    red |= Wire.read();
    red &= 0x3FFFF; // 18-bit mask
    
    // Read IR LED data (18-bit)
    uint32_t ir = 0;
    ir |= (uint32_t)Wire.read() << 16;
    ir |= (uint32_t)Wire.read() << 8;
    ir |= Wire.read();
    ir &= 0x3FFFF; // 18-bit mask
    
    data.red = red;
    data.ir = ir;
    data.valid = true;
  }
  
  return data;
}

void MAX30102_Driver::clearFIFO() {
  writeRegister(MAX30102_FIFO_WR_PTR, 0);
  writeRegister(MAX30102_FIFO_OVF_CNT, 0);
  writeRegister(MAX30102_FIFO_RD_PTR, 0);
}

float MAX30102_Driver::readTemperature() {
  // Trigger temperature reading
  writeRegister(MAX30102_TEMP_CONFIG, 0x01);
  
  // Wait for temperature conversion to complete
  delay(100);
  
  // Read temperature
  int8_t tempInt = readRegister(MAX30102_TEMP_INT);
  uint8_t tempFrac = readRegister(MAX30102_TEMP_FRAC);
  
  return (float)tempInt + ((float)tempFrac * 0.0625);
}

uint8_t MAX30102_Driver::getPartID() {
  return readRegister(MAX30102_PART_ID);
}

uint8_t MAX30102_Driver::getRevisionID() {
  return readRegister(MAX30102_REV_ID);
}

bool MAX30102_Driver::checkConnection() {
  uint8_t partID = getPartID();
  return (partID == 0x15); // MAX30102 Part ID
}

// Private helper functions
uint8_t MAX30102_Driver::readRegister(uint8_t reg) {
  Wire.beginTransmission(MAX30102_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(MAX30102_ADDRESS, 1);
  
  if (Wire.available()) {
    return Wire.read();
  }
  
  return 0;
}

void MAX30102_Driver::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MAX30102_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void MAX30102_Driver::bitMask(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t originalValue = readRegister(reg);
  uint8_t newValue = (originalValue & mask) | value;
  writeRegister(reg, newValue);
}

