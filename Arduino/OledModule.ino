#include "OledModule.h"

// 你可以依照螢幕實際大小修改這裡的預設參數
OledModule::OledModule(uint8_t width, uint8_t height)
  : _display(width, height, &Wire, -1), // -1表示不用重置腳位
    _initialized(false)
{
}

bool OledModule::begin(uint8_t i2cAddress, int sdaPin, int sclPin) {
  // 初始化 I2C
  Wire.begin(sdaPin, sclPin);

  // SSD1306_SWITCHCAPVCC 表示使用內部升壓電路
  if(!_display.begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
    // 若回傳 false 表示初始化失敗
    return false;
  }
  _display.clearDisplay();
  _display.display();
  _initialized = true;
  return true;
}

void OledModule::clear() {
  if(!_initialized) return;
  _display.clearDisplay();
}

void OledModule::drawText(const String &text, int16_t x, int16_t y, uint8_t size) {
  if(!_initialized) return;
  _display.setCursor(x, y);
  _display.setTextSize(size);
  _display.setTextColor(SSD1306_WHITE);
  _display.println(text);
}

void OledModule::display() {
  if(!_initialized) return;
  _display.display();
}
