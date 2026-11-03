#pragma once

#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP30.h>
#include "AppConfig.h"
#include "DeskWidgetTypes.h"
#include "StorageService.h"

namespace dw {
class EnvironmentalService {
public:
  void begin(StorageService &storage, bool timeValid, time_t nowUtc);
  void service(uint32_t now, bool timeValid, time_t nowUtc);
  const EnvironmentalSnapshot &snapshot() const { return snapshot_; }
  uint32_t failures() const { return failures_; }
  uint32_t deadlineMisses() const { return deadlineMisses_; }

private:
  static uint32_t absoluteHumidity(float temperatureC, float rh);
  void updateTvocTrend(uint16_t value, uint32_t now);
  void resetTvocTrend();
  uint16_t tvocSampleAtAge(uint16_t age) const;
  void tryRestoreBaseline(uint32_t now, bool timeValid, time_t nowUtc);
  void persistBaseline(bool timeValid, time_t nowUtc, uint32_t now);
  Adafruit_AHTX0 aht_;
  Adafruit_SGP30 sgp_;
  StorageService *storage_{nullptr};
  EnvironmentalSnapshot snapshot_;
  bool ahtReady_{false}, sgpReady_{false}, baselineRestored_{false};
  uint32_t nextAht_{0}, nextSgp_{0}, sgpStarted_{0}, nextBaselineSave_{0};
  uint32_t nextBaselineRestore_{0};
  uint8_t baselineRestoreAttempts_{0};
  uint16_t tvocHistory_[cfg::kTvocTrendHistorySamples]{};
  uint16_t tvocHistoryHead_{0}, tvocHistoryCount_{0};
  TvocTrend pendingTvocTrend_{TvocTrend::Collecting};
  uint32_t pendingTvocTrendSince_{0};
  uint32_t failures_{0}, deadlineMisses_{0};
};
} // namespace dw