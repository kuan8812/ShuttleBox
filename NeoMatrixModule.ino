#include "NeoMatrixModule.h"
#include <Adafruit_NeoPixel.h>

NeoMatrixModule::NeoMatrixModule(uint8_t pin, uint8_t width, uint8_t height)
  : _pin(pin), _width(width), _height(height), _matrix(nullptr)
{
}

NeoMatrixModule::~NeoMatrixModule() {
  if (_matrix) {
    delete _matrix;
    _matrix = nullptr;
  }
}

void NeoMatrixModule::begin() {
  // 建立 NeoMatrix 物件，設定矩陣排列方式
  // 下面的排列方式依照大部分 8x8 模組的接法，若有需要請調整 NEO_MATRIX_ 的旗標
  _matrix = new Adafruit_NeoMatrix(_width, _height, _pin,
                NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
                NEO_MATRIX_ROWS    + NEO_MATRIX_ZIGZAG,
                NEO_GRB            + NEO_KHZ800);
  _matrix->begin();
  _matrix->setTextWrap(false);
  _matrix->setBrightness(40);
  _matrix->fillScreen(0);
  _matrix->show();
}

void NeoMatrixModule::clear() {
  if (_matrix) {
    _matrix->fillScreen(0);
  }
}

void NeoMatrixModule::drawText(const char *text, int16_t x, int16_t y, uint16_t color) {
  if (_matrix) {
    _matrix->setCursor(x, y);
    _matrix->setTextColor(color);
    _matrix->print(text);
  }
}

void NeoMatrixModule::show() {
  if (_matrix) {
    _matrix->show();
  }
}

void NeoMatrixModule::setBrightness(uint8_t brightness) {
  if (_matrix) {
    _matrix->setBrightness(brightness);
  }
}
