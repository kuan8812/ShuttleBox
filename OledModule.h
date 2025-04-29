#ifndef OLEDMODULE_H
#define OLEDMODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OledModule {
public:
  // 建構子：可在這裡設定螢幕寬高或其他參數
  OledModule(uint8_t width = 128, uint8_t height = 64);

  // 初始化函式：傳入 I2C 位址 (預設 0x3C)，以及 SDA、SCL 腳位
  bool begin(uint8_t i2cAddress = 0x3C, int sdaPin = SDA, int sclPin = SCL);

  // 清除畫面
  void clear();

  // 顯示文字 (傳入要顯示的字串、座標、字體大小)
  void drawText(const String &text, int16_t x = 0, int16_t y = 0, uint8_t size = 1);

  // 將畫面更新到螢幕
  void display();

private:
  Adafruit_SSD1306 _display;
  bool _initialized;
};

#endif
