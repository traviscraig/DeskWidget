#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>

namespace dw {

class BoardServices {
public:
  bool begin();
  bool beginDisplay();
  Arduino_GFX *display() const { return canvas_; }
  void present();
  Arduino_XCA9554SWSPI *expander() const { return expander_; }
  bool displayReady() const { return displayReady_; }
  bool readBoardButton(uint8_t pin) const;
  void setBacklight(bool enabled);

private:
  Arduino_XCA9554SWSPI *expander_{nullptr};
  Arduino_ESP32RGBPanel *rgbPanel_{nullptr};
  Arduino_RGB_Display *gfx_{nullptr};
  Arduino_Canvas *canvas_{nullptr};
  bool displayReady_{false};
};

} // namespace dw