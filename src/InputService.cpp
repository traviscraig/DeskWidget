#include "InputService.h"

namespace dw {

bool InputService::begin(BoardServices &board) {
  board_ = &board;
  seesawReady_ = seesaw_.begin(cfg::kSeesawAddress);
  if (seesawReady_) {
    neoPixelReady_ = neoPixel_.begin(cfg::kSeesawAddress);
    if (neoPixelReady_) {
      neoPixel_.setBrightness(cfg::kSeesawNeoPixelBrightness);
      neoPixel_.clear();
      neoPixel_.show();
    } else {
      Serial.println("Encoder NeoPixel unavailable");
    }
    // NeoPixel begin performs a seesaw reset, so configure encoder GPIO and
    // establish its position only after both seesaw clients are initialized.
    seesaw_.pinMode(cfg::kSeesawButtonPin, INPUT_PULLUP);
    lastEncoder_ = seesaw_.getEncoderPosition();
  }
  nextPoll_ = millis();
  return seesawReady_;
}

void InputService::setTvocTrend(TvocTrend trend) {
  if (!cfg::kTvocTrendIndicatorEnabled) trend = TvocTrend::Unavailable;
  switch (trend) {
    case TvocTrend::Improving: targetRed_ = 0; targetGreen_ = 210; targetBlue_ = 210; break;
    case TvocTrend::Steady: targetRed_ = 0; targetGreen_ = 210; targetBlue_ = 35; break;
    case TvocTrend::Worsening: targetRed_ = 255; targetGreen_ = 135; targetBlue_ = 0; break;
    case TvocTrend::Collecting: targetRed_ = 210; targetGreen_ = 210; targetBlue_ = 210; break;
    default: targetRed_ = targetGreen_ = targetBlue_ = 0; break;
  }
}

void InputService::serviceNeoPixel(uint32_t now) {
  if (!neoPixelReady_ || !due(now, nextPixelTransition_)) return;
  nextPixelTransition_ = now + cfg::kSeesawNeoPixelTransitionMs;
  const auto approach = [](uint8_t value, uint8_t target) {
    if (value < target) return static_cast<uint8_t>(min<int>(target, value + cfg::kSeesawNeoPixelTransitionStep));
    if (value > target) return static_cast<uint8_t>(max<int>(target, value - cfg::kSeesawNeoPixelTransitionStep));
    return value;
  };
  const uint8_t red = approach(pixelRed_, targetRed_);
  const uint8_t green = approach(pixelGreen_, targetGreen_);
  const uint8_t blue = approach(pixelBlue_, targetBlue_);
  if (red == pixelRed_ && green == pixelGreen_ && blue == pixelBlue_) return;
  pixelRed_ = red; pixelGreen_ = green; pixelBlue_ = blue;
  neoPixel_.setPixelColor(0, pixelRed_, pixelGreen_, pixelBlue_);
  neoPixel_.show();
}

void InputService::push(InputType type, int16_t delta, uint32_t now) {
  if (count_ == 16) {
    ++overflows_;
    return;
  }
  queue_[tail_].type = type;
  queue_[tail_].delta = delta;
  queue_[tail_].atMs = now;
  tail_ = (tail_ + 1) % 16;
  ++count_;
}

bool InputService::pop(InputEvent &event) {
  if (!count_) return false;
  event = queue_[head_];
  head_ = (head_ + 1) % 16;
  --count_;
  return true;
}

void InputService::updateButton(ButtonState &state, bool pressed,
                                InputType shortType, InputType longType,
                                uint32_t now) {
  if (pressed != state.raw) {
    state.raw = pressed;
    state.changedMs = now;
  }
  if (state.stable != state.raw && now - state.changedMs >= cfg::kDebounceMs) {
    state.stable = state.raw;
    if (state.stable) {
      state.pressedMs = now;
      state.longSent = false;
    } else if (!state.longSent) {
      push(shortType, 0, now);
    }
  }
  if (state.stable && !state.longSent && now - state.pressedMs >= cfg::kLongPressMs) {
    state.longSent = true;
    push(longType, 0, now);
  }
}

void InputService::service(uint32_t now) {
  if (!due(now, nextPoll_)) return;
  nextPoll_ = now + cfg::kInputPollMs;
  serviceNeoPixel(now);
  if (seesawReady_) {
    const int32_t position = seesaw_.getEncoderPosition();
    const int32_t movement = (position - lastEncoder_) * cfg::kEncoderDirection;
    lastEncoder_ = position;
    if (movement) push(InputType::Rotation, constrain(movement, -127, 127), now);
    updateButton(select_, !seesaw_.digitalRead(cfg::kSeesawButtonPin),
                 InputType::Select, InputType::LongSelect, now);
  }
  updateButton(back_, board_->readBoardButton(PCA_BUTTON_UP),
               InputType::Back, InputType::LongBack, now);
  updateButton(menu_, board_->readBoardButton(PCA_BUTTON_DOWN),
               InputType::Menu, InputType::LongMenu, now);
}

} // namespace dw