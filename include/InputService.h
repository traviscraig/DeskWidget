#pragma once

#include <Adafruit_seesaw.h>
#include <seesaw_neopixel.h>
#include "AppConfig.h"
#include "BoardServices.h"
#include "DeskWidgetTypes.h"

namespace dw {

class InputService {
public:
  bool begin(BoardServices &board);
  void service(uint32_t now);
  bool pop(InputEvent &event);
  void setTvocTrend(TvocTrend trend);
  bool available() const { return seesawReady_; }
  uint32_t overflows() const { return overflows_; }

private:
  struct ButtonState {
    bool raw{false};
    bool stable{false};
    bool longSent{false};
    uint32_t changedMs{0};
    uint32_t pressedMs{0};
  };
  void updateButton(ButtonState &state, bool pressed, InputType shortType,
                    InputType longType, uint32_t now);
  void push(InputType type, int16_t delta, uint32_t now);
  void serviceNeoPixel(uint32_t now);

  BoardServices *board_{nullptr};
  Adafruit_seesaw seesaw_;
  seesaw_NeoPixel neoPixel_{1, cfg::kSeesawNeoPixelPin, NEO_GRB + NEO_KHZ800, &Wire};
  bool seesawReady_{false};
  bool neoPixelReady_{false};
  uint8_t pixelRed_{0}, pixelGreen_{0}, pixelBlue_{0};
  uint8_t targetRed_{0}, targetGreen_{0}, targetBlue_{0};
  uint32_t nextPixelTransition_{0};
  int32_t lastEncoder_{0};
  uint32_t nextPoll_{0};
  ButtonState select_, back_, menu_;
  InputEvent queue_[16]{};
  uint8_t head_{0}, tail_{0}, count_{0};
  uint32_t overflows_{0};
};

} // namespace dw