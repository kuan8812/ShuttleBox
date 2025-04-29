#include "FiberSensorModule.h"

FiberSensorModule::FiberSensorModule(uint8_t pinA, uint8_t pinB)
  : _pinA(pinA), _pinB(pinB)
{
}

void FiberSensorModule::begin() {
  // 這裡可依實際感測器需求決定是否要用 INPUT_PULLUP
  pinMode(_pinA, INPUT);
  pinMode(_pinB, INPUT);
}

int FiberSensorModule::readPinA() {
  return digitalRead(_pinA);
}

int FiberSensorModule::readPinB() {
  return digitalRead(_pinB);
}
