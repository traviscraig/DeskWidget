#include "EnvironmentalService.h"
#include "AppConfig.h"
#include <cmath>

namespace dw {
namespace {
template <typename T>
void updateMeasurement(Measurement<T> &measurement, T value, Quality quality,
                       uint32_t now, time_t nowUtc) {
  measurement.value = value;
  measurement.quality = quality;
  measurement.lastSuccessMs = now;
  measurement.utc = nowUtc;
}
}

void EnvironmentalService::begin(StorageService &storage, bool timeValid, time_t nowUtc) {
  storage_ = &storage;
  ahtReady_ = aht_.begin();
  sgpReady_ = sgp_.begin();
  const uint32_t now = millis();
  nextAht_ = nextSgp_ = now;
  sgpStarted_ = now;
  nextBaselineSave_ = now + cfg::kBaselineFirstSaveMs;
  if (!ahtReady_) snapshot_.temperatureC.quality = snapshot_.humidityPercent.quality = Quality::Fault;
  if (!sgpReady_) {
    snapshot_.tvocPpb.quality = snapshot_.eco2Ppm.quality = Quality::Fault;
    snapshot_.tvocTrend = TvocTrend::Unavailable;
  } else {
    snapshot_.tvocTrend = TvocTrend::Collecting;
  }
  tryRestoreBaseline(now, timeValid, nowUtc);
}

uint16_t EnvironmentalService::tvocSampleAtAge(uint16_t age) const {
  const int32_t newest = static_cast<int32_t>(tvocHistoryHead_) - 1;
  const int32_t index = newest - age;
  const int32_t size = cfg::kTvocTrendHistorySamples;
  return tvocHistory_[(index % size + size) % size];
}

void EnvironmentalService::resetTvocTrend() {
  tvocHistoryHead_ = 0;
  tvocHistoryCount_ = 0;
  pendingTvocTrend_ = TvocTrend::Collecting;
  pendingTvocTrendSince_ = 0;
  snapshot_.tvocCurrentAveragePpb = 0;
  snapshot_.tvocEarlierAveragePpb = 0;
  snapshot_.tvocTrend = TvocTrend::Unavailable;
}

void EnvironmentalService::updateTvocTrend(uint16_t value, uint32_t now) {
  tvocHistory_[tvocHistoryHead_] = value;
  tvocHistoryHead_ = (tvocHistoryHead_ + 1) % cfg::kTvocTrendHistorySamples;
  if (tvocHistoryCount_ < cfg::kTvocTrendHistorySamples) ++tvocHistoryCount_;

  if (tvocHistoryCount_ < cfg::kTvocTrendHistorySamples) {
    snapshot_.tvocTrend = TvocTrend::Collecting;
    pendingTvocTrend_ = TvocTrend::Collecting;
    pendingTvocTrendSince_ = now;
    return;
  }

  uint32_t currentSum = 0;
  uint32_t earlierSum = 0;
  for (uint16_t age = 0; age < cfg::kTvocTrendAverageSamples; ++age) {
    currentSum += tvocSampleAtAge(age);
    earlierSum += tvocSampleAtAge(cfg::kTvocTrendLookbackSamples + age);
  }
  const float currentAverage = static_cast<float>(currentSum) / cfg::kTvocTrendAverageSamples;
  const float earlierAverage = static_cast<float>(earlierSum) / cfg::kTvocTrendAverageSamples;
  snapshot_.tvocCurrentAveragePpb = currentAverage;
  snapshot_.tvocEarlierAveragePpb = earlierAverage;
  const float threshold = max(static_cast<float>(cfg::kTvocTrendAbsoluteThresholdPpb),
                              earlierAverage * cfg::kTvocTrendRelativeThreshold);
  const float difference = currentAverage - earlierAverage;
  const TvocTrend candidate = difference > threshold ? TvocTrend::Worsening :
      (difference < -threshold ? TvocTrend::Improving : TvocTrend::Steady);

  if (candidate == snapshot_.tvocTrend) {
    pendingTvocTrend_ = candidate;
    pendingTvocTrendSince_ = now;
  } else if (candidate != pendingTvocTrend_) {
    pendingTvocTrend_ = candidate;
    pendingTvocTrendSince_ = now;
  } else if (now - pendingTvocTrendSince_ >= cfg::kTvocTrendPersistenceMs) {
    snapshot_.tvocTrend = candidate;
  }
}

uint32_t EnvironmentalService::absoluteHumidity(float temperatureC, float rh) {
  const float g = 216.7F * ((rh / 100.0F) * 6.112F *
      exp((17.62F * temperatureC) / (243.12F + temperatureC)) /
      (273.15F + temperatureC));
  return static_cast<uint32_t>(max(0.0F, g * 1000.0F));
}

void EnvironmentalService::tryRestoreBaseline(uint32_t now, bool timeValid, time_t nowUtc) {
  if (baselineRestored_ || baselineRestoreAttempts_ >= cfg::kBaselineRestoreMaxAttempts ||
      !sgpReady_ || !timeValid || !storage_ || !storage_->available()) return;
  ++baselineRestoreAttempts_;
  nextBaselineRestore_ = now + cfg::kBaselineRestoreRetryMs;
  SgpBaselineRecord record{};
  if (!storage_->loadBaseline(record)) return;
  const bool serialMatches = record.serial[0] == sgp_.serialnumber[0] &&
      record.serial[1] == sgp_.serialnumber[1] && record.serial[2] == sgp_.serialnumber[2];
  constexpr time_t kMaxAge = 7 * 24 * 60 * 60;
  if (serialMatches && record.capturedUtc > 0 && nowUtc >= record.capturedUtc &&
      nowUtc - record.capturedUtc <= kMaxAge && sgp_.setIAQBaseline(record.eco2, record.tvoc)) {
    baselineRestored_ = true;
    snapshot_.sgpConditioning = false;
    nextBaselineSave_ = now + cfg::kBaselineSaveMs;
  }
}

void EnvironmentalService::persistBaseline(bool timeValid, time_t nowUtc, uint32_t now) {
  if (!timeValid || !storage_ || !storage_->available() || !sgpReady_) return;
  uint16_t eco2 = 0, tvoc = 0;
  if (!sgp_.getIAQBaseline(&eco2, &tvoc)) { ++failures_; return; }
  SgpBaselineRecord record;
  record.eco2 = eco2;
  record.tvoc = tvoc;
  record.serial[0] = sgp_.serialnumber[0];
  record.serial[1] = sgp_.serialnumber[1];
  record.serial[2] = sgp_.serialnumber[2];
  record.capturedUtc = nowUtc;
  record.conditioningSeconds = (now - sgpStarted_) / 1000;
  if (!storage_->saveBaseline(record)) ++failures_;
  nextBaselineSave_ = now + cfg::kBaselineSaveMs;
}

void EnvironmentalService::service(uint32_t now, bool timeValid, time_t nowUtc) {
  if (timeValid && !baselineRestored_ &&
      baselineRestoreAttempts_ < cfg::kBaselineRestoreMaxAttempts &&
      due(now, nextBaselineRestore_)) {
    tryRestoreBaseline(now, timeValid, nowUtc);
  }
  if (ahtReady_ && due(now, nextAht_)) {
    nextAht_ = now + cfg::kAhtIntervalMs;
    sensors_event_t humidity{}, temperature{};
    if (aht_.getEvent(&humidity, &temperature) && isfinite(temperature.temperature) &&
        isfinite(humidity.relative_humidity) && humidity.relative_humidity >= 0 &&
        humidity.relative_humidity <= 100) {
      updateMeasurement(snapshot_.temperatureC, temperature.temperature, Quality::Valid, now, nowUtc);
      updateMeasurement(snapshot_.humidityPercent, humidity.relative_humidity, Quality::Valid, now, nowUtc);
      ++snapshot_.revision;
    } else { ++failures_; }
  }
  if (sgpReady_ && due(now, nextSgp_)) {
    if (now - nextSgp_ > 250) ++deadlineMisses_;
    nextSgp_ = now + cfg::kSgpIntervalMs;
    const bool freshCompensation = snapshot_.humidityPercent.quality == Quality::Valid &&
        now - snapshot_.humidityPercent.lastSuccessMs <= cfg::kSensorStaleMs;
    snapshot_.humidityCompensated = freshCompensation &&
        sgp_.setHumidity(absoluteHumidity(snapshot_.temperatureC.value, snapshot_.humidityPercent.value));
    if (sgp_.IAQmeasure()) {
      const Quality q = now - sgpStarted_ < cfg::kSgpConditioningMs && !baselineRestored_
          ? Quality::Initializing : Quality::Valid;
      updateMeasurement(snapshot_.tvocPpb, sgp_.TVOC, q, now, nowUtc);
      updateMeasurement(snapshot_.eco2Ppm, sgp_.eCO2, q, now, nowUtc);
      snapshot_.sgpConditioning = q == Quality::Initializing;
      if (q == Quality::Valid) updateTvocTrend(sgp_.TVOC, now);
      else snapshot_.tvocTrend = TvocTrend::Collecting;
      ++snapshot_.revision;
    } else { ++failures_; }
  }
  if (snapshot_.temperatureC.lastSuccessMs && now - snapshot_.temperatureC.lastSuccessMs > cfg::kSensorStaleMs)
    snapshot_.temperatureC.quality = snapshot_.humidityPercent.quality = Quality::Stale;
  if (snapshot_.tvocPpb.lastSuccessMs && now - snapshot_.tvocPpb.lastSuccessMs > cfg::kSensorStaleMs) {
    snapshot_.tvocPpb.quality = snapshot_.eco2Ppm.quality = Quality::Stale;
    if (snapshot_.tvocTrend != TvocTrend::Unavailable) {
      resetTvocTrend();
      ++snapshot_.revision;
    }
  }
  if (due(now, nextBaselineSave_)) persistBaseline(timeValid, nowUtc, now);
}
} // namespace dw