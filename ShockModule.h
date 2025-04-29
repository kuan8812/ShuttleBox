#ifndef SHOCKMODULE_H
#define SHOCKMODULE_H

#include <Arduino.h>

class ShockModule {
public:
  ShockModule(uint8_t pin, unsigned long pulseInterval, unsigned long pulseDuration);
  
  void begin();
  void update();

  // ★ 新增：強制立即觸發電擊
  void forceTrigger();

private:
  uint8_t _pin;
  unsigned long _pulseInterval;
  unsigned long _pulseDuration;
  unsigned long _lastPulseTime;
  bool _pulseActive;
};

#endif
