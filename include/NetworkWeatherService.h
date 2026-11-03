#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "DeskWidgetTypes.h"

namespace dw {
class NetworkWeatherService {
public:
  bool begin(const Configuration &configuration);
  void applyConfiguration(const Configuration &configuration);
  void service(uint32_t now);
  NetworkState networkState() const;
  WeatherSnapshot weather() const;
  bool connected() const { return networkState() == NetworkState::Connected; }
  uint32_t failures() const;
  void requestRefresh();
  void requestWifiScan();
  WifiScanSnapshot wifiScan() const;
  bool takeLocationUpdate(Configuration &configuration);

private:
  enum class CommandType : uint8_t { Configure, Refresh, Scan };
  struct Command { CommandType type; Configuration configuration; };
  static void taskEntry(void *context);
  void taskLoop();
  void connectWifi(const Configuration &configuration);
  void scanWifi();
  bool fetchIpLocation(Configuration &configuration);
  void fetchWeather(const Configuration &configuration);
  bool parseIsoUtc(const char *value, time_t &result);
  QueueHandle_t commands_{nullptr};
  TaskHandle_t task_{nullptr};
  mutable SemaphoreHandle_t mutex_{nullptr};
  NetworkState state_{NetworkState::Unconfigured};
  WeatherSnapshot weather_;
  WifiScanSnapshot wifiScan_;
  Configuration locationUpdate_{};
  bool locationUpdatePending_{false};
  uint32_t failures_{0};
  uint32_t nextRefresh_{0};
  uint32_t nextRetry_{0};
  uint8_t retryIndex_{0};
};
} // namespace dw