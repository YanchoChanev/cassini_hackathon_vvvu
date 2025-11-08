#ifndef SOS_BUTTON_DRIVER_H
#define SOS_BUTTON_DRIVER_H

#include <Arduino.h>

// Button states
enum ButtonState {
  BUTTON_IDLE,
  BUTTON_PRESSED,
  BUTTON_RELEASED,
  BUTTON_LONG_PRESS,
  BUTTON_DOUBLE_PRESS
};

// Button event callback type
typedef void (*ButtonCallback)(ButtonState state);

class SOSButton_Driver {
private:
  uint8_t _pin;
  bool _pullupEnabled;
  bool _activeHigh;
  
  // Button state tracking
  bool _currentState;
  bool _lastState;
  unsigned long _lastDebounceTime;
  unsigned long _pressStartTime;
  unsigned long _lastPressTime;
  
  // Configuration
  unsigned long _debounceDelay;
  unsigned long _longPressThreshold;
  unsigned long _doublePressInterval;
  
  // Press tracking
  bool _isPressed;
  bool _longPressTriggered;
  uint8_t _pressCount;
  
  // Callback
  ButtonCallback _callback;
  
  // Helper functions
  bool readButtonState();
  
public:
  SOSButton_Driver(uint8_t pin, bool pullupEnabled = true, bool activeHigh = false);
  
  // Initialization
  void begin();
  
  // Configuration
  void setDebounceDelay(unsigned long ms);
  void setLongPressThreshold(unsigned long ms);
  void setDoublePressInterval(unsigned long ms);
  void setCallback(ButtonCallback callback);
  
  // State reading
  void update();
  bool isPressed();
  bool wasPressed();
  bool wasReleased();
  bool isLongPress();
  bool isDoublePress();
  
  // Get press duration
  unsigned long getPressDuration();
  
  // Reset state
  void reset();
};

#endif // SOS_BUTTON_DRIVER_H

