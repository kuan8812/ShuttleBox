#ifndef NEOMATRIXMODULE_H
#define NEOMATRIXMODULE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>

class NeoMatrixModule {
public:
  // 建構子：傳入控制矩陣的腳位，預設矩陣大小 8x8
  NeoMatrixModule(uint8_t pin, uint8_t width = 8, uint8_t height = 8);
  ~NeoMatrixModule();

  // 初始化矩陣
  void begin();

  // 清除畫面 (填滿黑色)
  void clear();

  // 在矩陣上指定位置繪製文字
  void drawText(const char *text, int16_t x = 0, int16_t y = 0, uint16_t color = 0xFFFF);

  // 更新顯示
  void show();

  // 設定亮度 (0~255)
  void setBrightness(uint8_t brightness);

  // 新增方法：填滿整個矩陣
  void fillScreen(uint16_t color);

  // 新增方法：取得顏色，透過內部 Adafruit_NeoMatrix 物件
  uint16_t Color(uint8_t r, uint8_t g, uint8_t b);

private:
  uint8_t _pin;
  uint8_t _width, _height;
  Adafruit_NeoMatrix* _matrix;
};

#endif
