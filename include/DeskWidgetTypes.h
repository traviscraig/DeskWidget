#pragma once

#include <Arduino.h>
#include <time.h>

namespace dw {

enum class Quality : uint8_t { Unavailable, Initializing, Valid, Stale, Fault };
enum class AppMode : uint8_t { Startup, Normal, Configuration, Degraded };
enum class Screen : uint8_t { Main, Settings, Units, Location, TimeZone, Wifi, Diagnostics, ResetConfirm };
enum class InputType : uint8_t { Rotation, Select, Back, Menu, LongSelect, LongBack, LongMenu };
enum class NetworkState : uint8_t { Unconfigured, Disconnected, Scanning, Connecting, Connected, RetryWait };
enum class WeatherStatus : uint8_t { Unavailable, Fetching, Valid, Stale, Fault };
enum class TemperatureUnit : uint8_t { Celsius, Fahrenheit };
enum class WifiUiMode : uint8_t { Networks, Password };
enum class TvocTrend : uint8_t { Unavailable, Collecting, Improving, Steady, Worsening };

template <typename T> struct Measurement {
  T value{};
  Quality quality{Quality::Unavailable};
  uint32_t lastSuccessMs{0};
  time_t utc{0};
};

struct EnvironmentalSnapshot {
  Measurement<float> temperatureC;
  Measurement<float> humidityPercent;
  Measurement<uint16_t> tvocPpb;
  Measurement<uint16_t> eco2Ppm;
  float tvocCurrentAveragePpb{0};
  float tvocEarlierAveragePpb{0};
  TvocTrend tvocTrend{TvocTrend::Unavailable};
  bool humidityCompensated{false};
  bool sgpConditioning{true};
  uint32_t revision{0};
};

struct TimeSnapshot {
  time_t utc{0};
  tm local{};
  bool valid{false};
  bool networkSynchronized{false};
  bool networkTimeFallback{false};
  bool rtcAvailable{false};
  char holiday[40]{};
  uint32_t revision{0};
};

struct WeatherSnapshot {
  char location[33]{};
  float currentC{0};
  float highC{0};
  float lowC{0};
  time_t sunriseUtc{0};
  time_t sunsetUtc{0};
  int16_t weatherCode{-1};
  WeatherStatus status{WeatherStatus::Unavailable};
  time_t observationUtc{0};
  time_t serverUtc{0};
  uint32_t fetchedMs{0};
  uint32_t revision{0};
  uint32_t configRevision{0};
  char error[48]{};
};

struct Configuration {
  uint16_t schemaVersion{1};
  uint32_t revision{1};
  TemperatureUnit units{TemperatureUnit::Fahrenheit};
  char location[33]{"Pensacola, FL"};
  double latitude{30.42131};
  double longitude{-87.21691};
  char timezone[48]{"CST6CDT,M3.2.0,M11.1.0"};
  uint8_t brightness{100};
  bool wifiEnabled{true};
  char ssid[33]{};
  char password[65]{};
};

struct WifiNetwork {
  char ssid[33]{};
  int32_t rssi{-127};
  bool secure{true};
};

struct WifiScanSnapshot {
  WifiNetwork networks[16]{};
  uint8_t count{0};
  bool scanning{false};
  bool failed{false};
  uint32_t revision{0};
};

struct HealthSnapshot {
  uint32_t uptimeSeconds{0};
  uint32_t freeHeap{0};
  uint32_t freePsram{0};
  uint32_t inputQueueOverflows{0};
  uint32_t sgpDeadlineMisses{0};
  uint32_t sensorFailures{0};
  uint32_t weatherFailures{0};
  uint32_t saveFailures{0};
};

struct InputEvent {
  InputType type{InputType::Rotation};
  int16_t delta{0};
  uint32_t atMs{0};
};

inline bool due(uint32_t now, uint32_t deadline) {
  return static_cast<int32_t>(now - deadline) >= 0;
}

inline float displayTemperature(float c, TemperatureUnit unit) {
  return unit == TemperatureUnit::Fahrenheit ? (c * 9.0F / 5.0F + 32.0F) : c;
}

} // namespace dw