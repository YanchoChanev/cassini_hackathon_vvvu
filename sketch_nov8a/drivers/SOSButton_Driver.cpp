#include "SOSButton_Driver.h"

SOSButton_Driver::SOSButton_Driver(uint8_t pin, bool pullupEnabled, bool activeHigh) {
  _pin = pin;
  _pullupEnabled = pullupEnabled;
  _activeHigh = activeHigh;
  
  // Default configuration
  _debounceDelay = 50;           // 50ms debounce
  _longPressThreshold = 2000;    // 2 seconds for long press
  _doublePressInterval = 400;    // 400ms for double press detection
  
  // Initialize state
  _currentState = false;
  _lastState = false;
  _lastDebounceTime = 0;
  _pressStartTime = 0;
  _lastPressTime = 0;
  _isPressed = false;
  _longPressTriggered = false;
  _pressCount = 0;
  _callback = nullptr;
}

void SOSButton_Driver::begin() {
  if (_pullupEnabled) {
    pinMode(_pin, INPUT_PULLUP);
  } else {
    pinMode(_pin, INPUT);
  }
  
  _lastState = readButtonState();
  _currentState = _lastState;
}

void SOSButton_Driver::setDebounceDelay(unsigned long ms) {
  _debounceDelay = ms;
}

void SOSButton_Driver::setLongPressThreshold(unsigned long ms) {
  _longPressThreshold = ms;
}

void SOSButton_Driver::setDoublePressInterval(unsigned long ms) {
  _doublePressInterval = ms;
}

void SOSButton_Driver::setCallback(ButtonCallback callback) {
  _callback = callback;
}

void SOSButton_Driver::update() {
  bool reading = readButtonState();
  unsigned long currentTime = millis();
  
  // Debouncing
  if (reading != _lastState) {
    _lastDebounceTime = currentTime;
  }
  
  if ((currentTime - _lastDebounceTime) > _debounceDelay) {
    // State has been stable for debounce period
    if (reading != _currentState) {
      _currentState = reading;
      
      // Button pressed
      if (_currentState) {
        _isPressed = true;
        _pressStartTime = currentTime;
        _longPressTriggered = false;
        
        // Check for double press
        if ((currentTime - _lastPressTime) < _doublePressInterval) {
          _pressCount = 2;
          if (_callback) {
            _callback(BUTTON_DOUBLE_PRESS);
          }
        } else {
          _pressCount = 1;
        }
        
        if (_callback && _pressCount == 1) {
          _callback(BUTTON_PRESSED);
        }
      }
      // Button released
      else {
        _isPressed = false;
        _lastPressTime = currentTime;
        
        if (_callback && !_longPressTriggered) {
          _callback(BUTTON_RELEASED);
        }
      }
    }
    
    // Check for long press while button is held
    if (_currentState && !_longPressTriggered) {
      if ((currentTime - _pressStartTime) >= _longPressThreshold) {
        _longPressTriggered = true;
        if (_callback) {
          _callback(BUTTON_LONG_PRESS);
        }
      }
    }
  }
  
  _lastState = reading;
}

bool SOSButton_Driver::isPressed() {
  return _currentState;
}

bool SOSButton_Driver::wasPressed() {
  return _isPressed && _currentState;
}

bool SOSButton_Driver::wasReleased() {
  return !_currentState && !_isPressed;
}

bool SOSButton_Driver::isLongPress() {
  return _longPressTriggered;
}

bool SOSButton_Driver::isDoublePress() {
  return _pressCount == 2;
}

unsigned long SOSButton_Driver::getPressDuration() {
  if (_currentState) {
    return millis() - _pressStartTime;
  }
  return 0;
}

void SOSButton_Driver::reset() {
  _currentState = false;
  _lastState = false;
  _isPressed = false;
  _longPressTriggered = false;
  _pressCount = 0;
  _pressStartTime = 0;
  _lastPressTime = 0;
}

// Private helper functions
bool SOSButton_Driver::readButtonState() {
  bool state = digitalRead(_pin);
  
  // Invert if using pull-up (button pressed = LOW)
  if (!_activeHigh) {
    state = !state;
  }
  
  return state;
}

