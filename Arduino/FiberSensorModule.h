#ifndef FIBERSENSORMODULE_H
#define FIBERSENSORMODULE_H

#include <Arduino.h>

class FiberSensorModule {
public:
  // 建構子：指定兩個感測腳位
  FiberSensorModule(uint8_t pinA, uint8_t pinB);

  // 初始化（設定腳位模式為 INPUT 或 INPUT_PULLUP）
  void begin();

  // 讀取每個腳位狀態
  int readPinA();
  int readPinB();

private:
  uint8_t _pinA;
  uint8_t _pinB;
};

#endif
