#pragma once

#include <RTClib.h>
#include "DeskWidgetTypes.h"

namespace dw {
class TimeService {
public:
  void begin(const Configuration &configuration);
  void service(uint32_t now, bool networkConnected, time_t networkFallbackUtc = 0);
  void applyConfiguration(const Configuration &configuration);
  const TimeSnapshot &snapshot() const { return snapshot_; }
  time_t utcNow() const;
private:
  bool plausible(time_t value) const;
  RTC_DS3231 rtc_;
  TimeSnapshot snapshot_;
  bool rtcReady_{false}, ntpStarted_{false};
  uint32_t nextTick_{0}, nextNtp_{0}, lastRtcCheck_{0};
  char timezone_[48]{"UTC0"};
};
} // namespace dw