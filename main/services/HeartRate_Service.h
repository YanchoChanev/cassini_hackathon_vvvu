#ifndef HEARTRATE_SERVICE_H
#define HEARTRATE_SERVICE_H

#include <Arduino.h>

// Configuration constants
#define HR_BUFFER_SIZE 100        // Number of samples to store for analysis
#define HR_MIN_BPM 40             // Minimum valid heart rate
#define HR_MAX_BPM 200            // Maximum valid heart rate
#define HR_SAMPLE_RATE 100        // Samples per second (must match MAX30102 config)
#define HR_FINGER_THRESHOLD 50000 // Minimum IR value to detect finger

// SpO2 calculation constants
#define SPO2_MIN 70               // Minimum valid SpO2 percentage
#define SPO2_MAX 100              // Maximum valid SpO2 percentage

struct HeartRateData {
  float heartRate;              // Heart rate in BPM
  float spO2;                   // Blood oxygen saturation percentage
  bool fingerDetected;          // Is finger on sensor?
  bool validReading;            // Is the reading valid?
  uint32_t lastBeatTime;        // Timestamp of last detected beat
  float signalQuality;          // Signal quality indicator (0-100%)
};

class HeartRate_Service {
private:
  // Circular buffers for RED and IR samples
  uint32_t _redBuffer[HR_BUFFER_SIZE];
  uint32_t _irBuffer[HR_BUFFER_SIZE];
  uint8_t _bufferIndex;
  bool _bufferFull;
  
  // Heart rate detection
  float _lastHeartRate;
  uint32_t _lastBeatTime;
  uint32_t _beatInterval;
  bool _beatDetected;
  
  // Peak detection
  uint32_t _lastPeakValue;
  uint32_t _lastValleyValue;
  bool _peakDetected;
  uint8_t _peakCount;
  
  // SpO2 calculation
  float _lastSpO2;
  float _redAC;
  float _redDC;
  float _irAC;
  float _irDC;
  
  // Signal quality
  float _signalQuality;
  
  // Helper functions
  void updateBuffers(uint32_t red, uint32_t ir);
  bool detectPeak();
  float calculateHeartRate();
  float calculateSpO2();
  float calculateSignalQuality();
  bool isFingerDetected();
  
  // Signal processing helpers
  uint32_t getBufferAverage(uint32_t* buffer);
  uint32_t getBufferMax(uint32_t* buffer);
  uint32_t getBufferMin(uint32_t* buffer);
  float getBufferStdDev(uint32_t* buffer, uint32_t mean);
  
public:
  HeartRate_Service();
  
  // Initialize the service
  void begin();
  
  // Add new sample and process
  void addSample(uint32_t red, uint32_t ir);
  
  // Get current readings
  HeartRateData getReadings();
  
  // Reset all data
  void reset();
  
  // Check if enough data collected
  bool isReady();
};

#endif // HEARTRATE_SERVICE_H

