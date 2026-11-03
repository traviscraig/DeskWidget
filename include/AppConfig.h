#pragma once

#include <Arduino.h>

namespace dw::cfg {
// The ESP-IDF 4.4 RGB driver scans the framebuffer directly from PSRAM and
// cannot use the newer bounce-buffer option. 12 MHz leaves enough PSRAM bus
// margin for a completed back-buffer frame copy without transient line shifts.
constexpr uint32_t kDisplayPixelClockHz = 12000000UL;
constexpr bool kDisplayPclkActiveNeg = true;
constexpr uint16_t kDisplayHsyncFrontPorch = 40;
constexpr uint16_t kDisplayHsyncPulseWidth = 20;
constexpr uint16_t kDisplayHsyncBackPorch = 40;
constexpr uint16_t kDisplayVsyncFrontPorch = 40;
constexpr uint16_t kDisplayVsyncPulseWidth = 10;
constexpr uint16_t kDisplayVsyncBackPorch = 40;
constexpr uint8_t kEepromAddress = 0x50;
constexpr uint16_t kEepromSize = 4096;
constexpr uint16_t kConfigSlotA = 0;
constexpr uint16_t kConfigSlotB = 1024;
constexpr uint16_t kConfigSlotSize = 1024;
constexpr uint16_t kBaselineSlotA = 2048;
constexpr uint16_t kBaselineSlotB = 2304;
constexpr uint16_t kBaselineSlotSize = 256;
constexpr uint32_t kConfigMagic = 0x44574346UL; // DWCF
constexpr uint32_t kBaselineMagic = 0x4457424CUL; // DWBL
constexpr uint32_t kInputPollMs = 15;
constexpr uint32_t kDisplayMinFrameIntervalMs = 50;
constexpr uint32_t kDebounceMs = 30;
constexpr uint32_t kLongPressMs = 800;
constexpr uint32_t kAhtIntervalMs = 2000;
constexpr uint32_t kSgpIntervalMs = 1000;
constexpr uint32_t kSensorStaleMs = 10000;
constexpr uint32_t kClockIntervalMs = 1000;
constexpr uint32_t kWeatherIntervalMs = 10UL * 60UL * 1000UL;
constexpr uint32_t kWeatherStaleMs = 30UL * 60UL * 1000UL;
constexpr char kIpLocationUrl[] =
    "https://ipwho.is/?fields=success,message,city,region,region_code,country_code,latitude,longitude,timezone";
constexpr uint32_t kNtpIntervalMs = 6UL * 60UL * 60UL * 1000UL;
constexpr uint32_t kNtpRetryMs = 60UL * 1000UL;
constexpr uint32_t kSgpConditioningMs = 15UL * 1000UL;
constexpr uint32_t kBaselineFirstSaveMs = 12UL * 60UL * 60UL * 1000UL;
constexpr uint32_t kBaselineSaveMs = 60UL * 60UL * 1000UL;
constexpr uint32_t kBaselineRestoreRetryMs = 60UL * 1000UL;
constexpr uint8_t kBaselineRestoreMaxAttempts = 5;
constexpr uint8_t kSeesawAddress = 0x36;
constexpr uint8_t kSeesawButtonPin = 24;
constexpr int8_t kEncoderDirection = -1;
constexpr bool kTvocTrendIndicatorEnabled = true;
constexpr uint16_t kTvocTrendAverageSamples = 60;
constexpr uint16_t kTvocTrendLookbackSamples = 300;
constexpr uint16_t kTvocTrendHistorySamples = kTvocTrendAverageSamples + kTvocTrendLookbackSamples;
constexpr uint16_t kTvocTrendAbsoluteThresholdPpb = 25;
constexpr float kTvocTrendRelativeThreshold = 0.20F;
constexpr uint32_t kTvocTrendPersistenceMs = 30UL * 1000UL;
constexpr uint8_t kSeesawNeoPixelPin = 6;
constexpr uint8_t kSeesawNeoPixelBrightness = 24; // 0-255; intentionally dim
constexpr uint32_t kSeesawNeoPixelTransitionMs = 50;
constexpr uint8_t kSeesawNeoPixelTransitionStep = 4;
constexpr uint8_t kMaxWifiScanResults = 16;
} // namespace dw::cfg