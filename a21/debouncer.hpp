//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>
#include <a21/pins.hpp>

namespace a21 {
  
/** 
 * Simple debouncer logic.
 * TODO: pass the clock template from the outside
 */
template<typename T, int timeout_ms = 10, bool initial_value = false>
class Debouncer {

private:

  // The debounced value.
  volatile bool _value;

  // True, if the new value along and its timestamp are known but held till timeout elapses.
  volatile bool _holding;

  // The new value being held.
  volatile bool _heldValue;

  // When the new value was read.
  volatile unsigned long _timestamp;
  
public:

  Debouncer() 
    : _value(initial_value), _holding(false) {}

  /** Debounced value. */
  bool value() const {
    return _value;
  }

  /** Called when the debounced value changes. Interrupts are enabled. */
  void valueDidChange() { 
  }

  /** Should be called every time a raw value is read, for example from a pin interrupt handler. */
  void setValue(bool value) {
    if (!_holding || value != _heldValue) {
      _holding = true;
      _heldValue = value;
      _timestamp = millis();
    }
  }

  /** 
   * Should be called periodically (from the main loop) to check if the new value being held has finally settled. 
   * Note that it assumes that interrupts are enabled.
   */
  bool check() {
    
    noInterrupts();

    if (_holding && (int)(millis() - _timestamp) >= timeout_ms) { 
      
      // OK, enough time has passed, let's proclaim the value being held as a debounced one.
      _holding = false;
      
      bool changed = (_heldValue != _value);
      
      _value = _heldValue;
      
      interrupts();
      
      if (changed) {
        static_cast<T*>(this)->valueDidChange();
      }
      
    } else {
      interrupts();
    }
  }
};

template<typename Pin, int timeout_ms = 10, bool initial_value = false>
class DebouncedPin {
private:
  
  class DebouncerImp : public Debouncer<DebouncerImp, timeout_ms, initial_value> {} _debouncer;  
  
public:
  
  Pin pin;
    
  bool read() {
    _debouncer.setValue(pin.read());
    _debouncer.check();
    return _debouncer.value();
  }
};

} // namespace
