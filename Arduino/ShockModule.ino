#include "ShockModule.h"

ShockModule::ShockModule(uint8_t pin, unsigned long pulseInterval, unsigned long pulseDuration)
  : _pin(pin), _pulseInterval(pulseInterval), _pulseDuration(pulseDuration),
    _lastPulseTime(0), _pulseActive(false) {
}

void ShockModule::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void ShockModule::update() {
  unsigned long now = millis();

  // 若電擊未啟動，且已達到觸發間隔，則啟動電擊
  if (!_pulseActive && (now - _lastPulseTime >= _pulseInterval)) {
    digitalWrite(_pin, HIGH);
    _pulseActive = true;
    _lastPulseTime = now;
  }
  
  // 若電擊已啟動且達到持續時間，則關閉電擊
  if (_pulseActive && (now - _lastPulseTime >= _pulseDuration)) {
    digitalWrite(_pin, LOW);
    _pulseActive = false;
    // 此處 _lastPulseTime 已更新為電擊啟動時間
  }
}

// ★ 新增：強制立即啟動電擊
void ShockModule::forceTrigger() {
  // 直接拉高、標記為啟動中，並重置開始時間
  digitalWrite(_pin, HIGH);
  _pulseActive = true;
  _lastPulseTime = millis();  
}
