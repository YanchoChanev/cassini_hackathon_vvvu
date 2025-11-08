#include "HeartRate_Service.h"

HeartRate_Service::HeartRate_Service() {
  _bufferIndex = 0;
  _bufferFull = false;
  _lastHeartRate = 0;
  _lastBeatTime = 0;
  _beatInterval = 0;
  _beatDetected = false;
  _lastPeakValue = 0;
  _lastValleyValue = 0;
  _peakDetected = false;
  _peakCount = 0;
  _lastSpO2 = 0;
  _redAC = 0;
  _redDC = 0;
  _irAC = 0;
  _irDC = 0;
  _signalQuality = 0;
}

void HeartRate_Service::begin() {
  reset();
}

void HeartRate_Service::reset() {
  // Clear buffers
  for (int i = 0; i < HR_BUFFER_SIZE; i++) {
    _redBuffer[i] = 0;
    _irBuffer[i] = 0;
  }
  
  _bufferIndex = 0;
  _bufferFull = false;
  _lastHeartRate = 0;
  _lastBeatTime = 0;
  _beatInterval = 0;
  _beatDetected = false;
  _lastPeakValue = 0;
  _lastValleyValue = 0;
  _peakDetected = false;
  _peakCount = 0;
  _lastSpO2 = 0;
  _signalQuality = 0;
}

void HeartRate_Service::addSample(uint32_t red, uint32_t ir) {
  // Update circular buffers
  updateBuffers(red, ir);
  
  // Only process if we have enough data
  if (!_bufferFull) {
    return;
  }
  
  // Detect peaks for heart rate
  if (detectPeak()) {
    _lastHeartRate = calculateHeartRate();
  }
  
  // Calculate SpO2
  _lastSpO2 = calculateSpO2();
  
  // Calculate signal quality
  _signalQuality = calculateSignalQuality();
}

HeartRateData HeartRate_Service::getReadings() {
  HeartRateData data;
  
  data.fingerDetected = isFingerDetected();
  data.validReading = _bufferFull && data.fingerDetected && (_signalQuality > 30);
  data.heartRate = _lastHeartRate;
  data.spO2 = _lastSpO2;
  data.lastBeatTime = _lastBeatTime;
  data.signalQuality = _signalQuality;
  
  return data;
}

bool HeartRate_Service::isReady() {
  return _bufferFull;
}

// Private helper functions

void HeartRate_Service::updateBuffers(uint32_t red, uint32_t ir) {
  _redBuffer[_bufferIndex] = red;
  _irBuffer[_bufferIndex] = ir;
  
  _bufferIndex++;
  
  if (_bufferIndex >= HR_BUFFER_SIZE) {
    _bufferIndex = 0;
    _bufferFull = true;
  }
}

bool HeartRate_Service::detectPeak() {
  if (!_bufferFull) {
    return false;
  }
  
  // Use IR signal for peak detection (more stable than RED)
  uint32_t currentValue = _irBuffer[(_bufferIndex - 1 + HR_BUFFER_SIZE) % HR_BUFFER_SIZE];
  uint32_t previousValue = _irBuffer[(_bufferIndex - 2 + HR_BUFFER_SIZE) % HR_BUFFER_SIZE];
  uint32_t beforePrevious = _irBuffer[(_bufferIndex - 3 + HR_BUFFER_SIZE) % HR_BUFFER_SIZE];
  
  // Calculate average and threshold
  uint32_t avgValue = getBufferAverage(_irBuffer);
  uint32_t threshold = avgValue * 1.05; // 5% above average
  
  // Detect peak: previous value is higher than both neighbors and above threshold
  if (previousValue > currentValue && 
      previousValue > beforePrevious && 
      previousValue > threshold) {
    
    // Avoid detecting the same peak multiple times
    if (!_peakDetected) {
      _peakDetected = true;
      _lastPeakValue = previousValue;
      _peakCount++;
      
      // Record beat time
      uint32_t currentTime = millis();
      if (_lastBeatTime > 0) {
        _beatInterval = currentTime - _lastBeatTime;
      }
      _lastBeatTime = currentTime;
      
      return true;
    }
  } else {
    _peakDetected = false;
  }
  
  return false;
}

float HeartRate_Service::calculateHeartRate() {
  if (_beatInterval == 0 || _beatInterval < 300 || _beatInterval > 1500) {
    // Invalid interval (too fast or too slow)
    return _lastHeartRate; // Return last valid reading
  }
  
  // Calculate BPM from interval
  float bpm = 60000.0 / (float)_beatInterval;
  
  // Validate range
  if (bpm < HR_MIN_BPM || bpm > HR_MAX_BPM) {
    return _lastHeartRate; // Return last valid reading
  }
  
  // Apply smoothing (exponential moving average)
  if (_lastHeartRate > 0) {
    bpm = (_lastHeartRate * 0.7) + (bpm * 0.3);
  }
  
  return bpm;
}

float HeartRate_Service::calculateSpO2() {
  if (!_bufferFull) {
    return 0;
  }
  
  // Calculate DC (average) and AC (variation) for both RED and IR
  _redDC = getBufferAverage(_redBuffer);
  _irDC = getBufferAverage(_irBuffer);
  
  uint32_t redMax = getBufferMax(_redBuffer);
  uint32_t redMin = getBufferMin(_redBuffer);
  uint32_t irMax = getBufferMax(_irBuffer);
  uint32_t irMin = getBufferMin(_irBuffer);
  
  _redAC = (float)(redMax - redMin);
  _irAC = (float)(irMax - irMin);
  
  // Avoid division by zero
  if (_redDC == 0 || _irDC == 0 || _irAC == 0) {
    return _lastSpO2;
  }
  
  // Calculate R value (ratio of ratios)
  float R = (_redAC / _redDC) / (_irAC / _irDC);
  
  // SpO2 calculation using empirical formula
  // This is a simplified version - actual calibration may be needed
  float spO2 = 110.0 - 25.0 * R;
  
  // Clamp to valid range
  if (spO2 < SPO2_MIN) spO2 = SPO2_MIN;
  if (spO2 > SPO2_MAX) spO2 = SPO2_MAX;
  
  // Apply smoothing
  if (_lastSpO2 > 0) {
    spO2 = (_lastSpO2 * 0.8) + (spO2 * 0.2);
  }
  
  return spO2;
}

float HeartRate_Service::calculateSignalQuality() {
  if (!_bufferFull) {
    return 0;
  }
  
  // Signal quality based on:
  // 1. Signal strength (IR DC value)
  // 2. Signal variation (IR AC value)
  // 3. Signal stability (standard deviation)
  
  uint32_t irAvg = getBufferAverage(_irBuffer);
  float irStdDev = getBufferStdDev(_irBuffer, irAvg);
  
  // Strength score (0-100)
  float strengthScore = 0;
  if (irAvg > HR_FINGER_THRESHOLD) {
    strengthScore = min(100.0, (float)irAvg / 2000.0);
  }
  
  // Variation score (0-100) - we want some variation for heartbeat
  float variationScore = 0;
  if (_irAC > 100) {
    variationScore = min(100.0, _irAC / 100.0);
  }
  
  // Stability score (0-100) - lower std dev is better
  float stabilityScore = 100.0 - min(100.0, irStdDev / 100.0);
  
  // Combined quality score
  float quality = (strengthScore * 0.5) + (variationScore * 0.3) + (stabilityScore * 0.2);
  
  return quality;
}

bool HeartRate_Service::isFingerDetected() {
  if (!_bufferFull) {
    return false;
  }
  
  uint32_t irAvg = getBufferAverage(_irBuffer);
  return (irAvg > HR_FINGER_THRESHOLD);
}

// Signal processing helpers

uint32_t HeartRate_Service::getBufferAverage(uint32_t* buffer) {
  uint64_t sum = 0;
  for (int i = 0; i < HR_BUFFER_SIZE; i++) {
    sum += buffer[i];
  }
  return (uint32_t)(sum / HR_BUFFER_SIZE);
}

uint32_t HeartRate_Service::getBufferMax(uint32_t* buffer) {
  uint32_t maxVal = 0;
  for (int i = 0; i < HR_BUFFER_SIZE; i++) {
    if (buffer[i] > maxVal) {
      maxVal = buffer[i];
    }
  }
  return maxVal;
}

uint32_t HeartRate_Service::getBufferMin(uint32_t* buffer) {
  uint32_t minVal = 0xFFFFFFFF;
  for (int i = 0; i < HR_BUFFER_SIZE; i++) {
    if (buffer[i] < minVal) {
      minVal = buffer[i];
    }
  }
  return minVal;
}

float HeartRate_Service::getBufferStdDev(uint32_t* buffer, uint32_t mean) {
  float variance = 0;
  for (int i = 0; i < HR_BUFFER_SIZE; i++) {
    float diff = (float)buffer[i] - (float)mean;
    variance += diff * diff;
  }
  variance /= HR_BUFFER_SIZE;
  return sqrt(variance);
}

