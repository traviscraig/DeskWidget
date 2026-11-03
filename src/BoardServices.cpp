#include "BoardServices.h"
#include "AppConfig.h"

namespace dw {
namespace {

class PsramCanvas final : public Arduino_Canvas {
public:
  PsramCanvas(int16_t width, int16_t height, Arduino_G *output)
      : Arduino_Canvas(width, height, output) {}

  bool begin(int32_t speed = GFX_NOT_DEFINED) override {
    if (!_framebuffer) {
      const size_t bytes = static_cast<size_t>(_width) * _height * sizeof(uint16_t);
      _framebuffer = static_cast<uint16_t *>(ps_malloc(bytes));
      if (!_framebuffer) return false;
    }
    return Arduino_Canvas::begin(GFX_SKIP_OUTPUT_BEGIN);
  }
};

} // namespace

bool BoardServices::begin() {
  Wire.begin(SDA, SCL);
  Wire.setClock(400000);
  return beginDisplay();
}

bool BoardServices::beginDisplay() {
  expander_ = new Arduino_XCA9554SWSPI(PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK,
                                       PCA_TFT_MOSI, &Wire, 0x3F);
  rgbPanel_ = new Arduino_ESP32RGBPanel(
      TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
      TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
      TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
      TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,
      1, cfg::kDisplayHsyncFrontPorch, cfg::kDisplayHsyncPulseWidth,
      cfg::kDisplayHsyncBackPorch,
      1, cfg::kDisplayVsyncFrontPorch, cfg::kDisplayVsyncPulseWidth,
      cfg::kDisplayVsyncBackPorch,
      cfg::kDisplayPclkActiveNeg, cfg::kDisplayPixelClockHz, false);
  gfx_ = new Arduino_RGB_Display(
      480, 480, rgbPanel_, 0, false, expander_, GFX_NOT_DEFINED,
      tl034wvs05_b1477a_init_operations,
      sizeof(tl034wvs05_b1477a_init_operations));

  if (!expander_ || !rgbPanel_ || !gfx_ || !psramFound()) {
    return false;
  }
  if (!gfx_->begin()) {
    return false;
  }
  canvas_ = new PsramCanvas(480, 480, gfx_);
  if (!canvas_ || !canvas_->begin()) {
    return false;
  }
  expander_->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander_->pinMode(PCA_BUTTON_UP, INPUT_PULLUP);
  expander_->pinMode(PCA_BUTTON_DOWN, INPUT_PULLUP);
  gfx_->fillScreen(BLACK);
  gfx_->flush();
  setBacklight(true);
  displayReady_ = true;
  return true;
}

void BoardServices::present() {
  if (!canvas_ || !gfx_) return;
  canvas_->flush();
  gfx_->flush();
}

bool BoardServices::readBoardButton(uint8_t pin) const {
  return expander_ ? expander_->digitalRead(pin) == LOW : false;
}

void BoardServices::setBacklight(bool enabled) {
  if (expander_) {
    expander_->digitalWrite(PCA_TFT_BACKLIGHT, enabled ? HIGH : LOW);
  }
}

} // namespace dw