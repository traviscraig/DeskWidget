#include "TimeService.h"
#include "AppConfig.h"
#include "HolidayService.h"
#include <sys/time.h>

namespace dw {
bool TimeService::plausible(time_t value) const {
  // Keep the bounds wider than time_t. On ESP32 toolchains where time_t is a
  // signed 32-bit value, storing 2100-01-01 directly in time_t overflows and
  // causes every current timestamp to be rejected.
  constexpr int64_t earliest = 1577836800LL; // 2020-01-01 UTC
  constexpr int64_t latest = 4102444800LL;   // 2100-01-01 UTC
  const int64_t seconds = static_cast<int64_t>(value);
  return seconds >= earliest && seconds < latest;
}

void TimeService::applyConfiguration(const Configuration &configuration) {
  strlcpy(timezone_, configuration.timezone, sizeof(timezone_));
  setenv("TZ", timezone_, 1); tzset();
}

void TimeService::begin(const Configuration &configuration) {
  applyConfiguration(configuration);
  rtcReady_ = rtc_.begin();
  snapshot_.rtcAvailable = rtcReady_;
  if (rtcReady_ && !rtc_.lostPower()) {
    const time_t rtcUtc = rtc_.now().unixtime();
    if (plausible(rtcUtc)) {
      timeval tv{rtcUtc, 0};
      if (settimeofday(&tv, nullptr) == 0) {
        snapshot_.valid = true;
        Serial.printf("Time initialized from RTC: %lld UTC\n", static_cast<long long>(rtcUtc));
      } else {
        Serial.println("RTC time valid but settimeofday failed");
      }
    }
  }
  nextTick_ = millis(); nextNtp_ = millis();
}

time_t TimeService::utcNow() const { return time(nullptr); }

void TimeService::service(uint32_t now, bool networkConnected, time_t networkFallbackUtc) {
  if (networkConnected && (!ntpStarted_ || due(now, nextNtp_))) {
    configTzTime(timezone_, "pool.ntp.org", "time.nist.gov");
    ntpStarted_ = true;
    nextNtp_ = now + (snapshot_.valid ? cfg::kNtpIntervalMs : cfg::kNtpRetryMs);
  }
  if (!due(now, nextTick_)) return;
  nextTick_ = now + cfg::kClockIntervalMs;
  time_t current = utcNow();
  if (!plausible(current) && plausible(networkFallbackUtc)) {
    timeval tv{networkFallbackUtc, 0};
    if (settimeofday(&tv, nullptr) == 0) {
      current = utcNow();
      snapshot_.networkTimeFallback = true;
      Serial.printf("Time initialized from weather network timestamp: %lld UTC\n",
                    static_cast<long long>(networkFallbackUtc));
      if (rtcReady_) {
        rtc_.adjust(DateTime(static_cast<uint32_t>(networkFallbackUtc)));
        Serial.println("RTC updated from weather network timestamp");
      }
    } else {
      Serial.println("Weather network time valid but settimeofday failed");
    }
  }
  if (plausible(current)) {
    snapshot_.valid = true; snapshot_.utc = current;
    localtime_r(&current, &snapshot_.local);
    HolidayService::labelForDate(snapshot_.local, snapshot_.holiday, sizeof(snapshot_.holiday));
    if (snapshot_.networkSynchronized && rtcReady_ && now - lastRtcCheck_ > 60000) {
      lastRtcCheck_ = now;
      const time_t rtcUtc = rtc_.now().unixtime();
      if (!plausible(rtcUtc) || llabs(static_cast<long long>(rtcUtc) - current) >= 2)
        rtc_.adjust(DateTime(static_cast<uint32_t>(current)));
    }
  }
  ++snapshot_.revision;
}
} // namespace dw