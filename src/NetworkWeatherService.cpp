#include "NetworkWeatherService.h"
#include "AppConfig.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

namespace dw {
namespace {
// Public ISRG Root X1 trust anchor used by the Open-Meteo HTTPS service.
static const char kRootCa[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// GlobalSign ECC Root CA - R4, used by the ipwho.is WE1 certificate chain.
static const char kIpLocationRootCa[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIB3DCCAYOgAwIBAgINAgPlfvU/k/2lCSGypjAKBggqhkjOPQQDAjBQMSQwIgYD
VQQLExtHbG9iYWxTaWduIEVDQyBSb290IENBIC0gUjQxEzARBgNVBAoTCkdsb2Jh
bFNpZ24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMTIxMTEzMDAwMDAwWhcNMzgw
MTE5MDMxNDA3WjBQMSQwIgYDVQQLExtHbG9iYWxTaWduIEVDQyBSb290IENBIC0g
UjQxEzARBgNVBAoTCkdsb2JhbFNpZ24xEzARBgNVBAMTCkdsb2JhbFNpZ24wWTAT
BgcqhkjOPQIBBggqhkjOPQMBBwNCAAS4xnnTj2wlDp8uORkcA6SumuU5BwkWymOx
uYb4ilfBV85C+nOh92VC/x7BALJucw7/xyHlGKSq2XE/qNS5zowdo0IwQDAOBgNV
HQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUVLB7rUW44kB/
+wpu+74zyTyjhNUwCgYIKoZIzj0EAwIDRwAwRAIgIk90crlgr/HmnKAWBVBFw147
bmF0774BxL4YSFlhgjICICadVGNA3jdgUM/I2O2dgq43mLyjj0xMqTQrbO/7lZsm
-----END CERTIFICATE-----
)EOF";

void setState(SemaphoreHandle_t mutex, NetworkState &target, NetworkState value) {
  xSemaphoreTake(mutex, portMAX_DELAY); target = value; xSemaphoreGive(mutex);
}

int64_t daysFromCivil(int year, unsigned month, unsigned day) {
  year -= month <= 2;
  const int era = (year >= 0 ? year : year - 399) / 400;
  const unsigned yearOfEra = static_cast<unsigned>(year - era * 400);
  const unsigned dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const unsigned dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
  return era * 146097LL + static_cast<int64_t>(dayOfEra) - 719468LL;
}

bool timezoneRule(const char *iana, int32_t utcOffsetSeconds, char *rule, size_t capacity) {
  struct Mapping { const char *iana; const char *posix; };
  static const Mapping mappings[] = {
      {"America/New_York", "EST5EDT,M3.2.0,M11.1.0"},
      {"America/Detroit", "EST5EDT,M3.2.0,M11.1.0"},
      {"America/Indiana/Indianapolis", "EST5EDT,M3.2.0,M11.1.0"},
      {"America/Kentucky/Louisville", "EST5EDT,M3.2.0,M11.1.0"},
      {"America/Chicago", "CST6CDT,M3.2.0,M11.1.0"},
      {"America/Indiana/Knox", "CST6CDT,M3.2.0,M11.1.0"},
      {"America/Denver", "MST7MDT,M3.2.0,M11.1.0"},
      {"America/Boise", "MST7MDT,M3.2.0,M11.1.0"},
      {"America/Phoenix", "MST7"},
      {"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"},
      {"America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0"},
      {"America/Adak", "HST10HDT,M3.2.0,M11.1.0"},
      {"Pacific/Honolulu", "HST10"},
      {"America/Puerto_Rico", "AST4"},
      {"America/Toronto", "EST5EDT,M3.2.0,M11.1.0"},
      {"America/Winnipeg", "CST6CDT,M3.2.0,M11.1.0"},
      {"America/Edmonton", "MST7MDT,M3.2.0,M11.1.0"},
      {"America/Vancouver", "PST8PDT,M3.2.0,M11.1.0"},
      {"America/Halifax", "AST4ADT,M3.2.0,M11.1.0"},
      {"America/St_Johns", "NST3:30NDT,M3.2.0,M11.1.0"},
  };
  for (const auto &mapping : mappings) {
    if (strcmp(iana, mapping.iana) == 0) {
      strlcpy(rule, mapping.posix, capacity);
      return true;
    }
  }

  if (utcOffsetSeconds < -14 * 3600 || utcOffsetSeconds > 14 * 3600) return false;
  const int32_t posixMinutes = -utcOffsetSeconds / 60;
  const int32_t hours = abs(posixMinutes) / 60;
  const int32_t minutes = abs(posixMinutes) % 60;
  if (!posixMinutes) snprintf(rule, capacity, "UTC0");
  else if (minutes) snprintf(rule, capacity, "UTC%s%ld:%02ld", posixMinutes < 0 ? "-" : "",
                             static_cast<long>(hours), static_cast<long>(minutes));
  else snprintf(rule, capacity, "UTC%s%ld", posixMinutes < 0 ? "-" : "", static_cast<long>(hours));
  return true;
}

bool locationChanged(const Configuration &a, const Configuration &b) {
  return strcmp(a.location, b.location) != 0 || strcmp(a.timezone, b.timezone) != 0 ||
         fabs(a.latitude - b.latitude) >= 0.0001 || fabs(a.longitude - b.longitude) >= 0.0001;
}
}

bool NetworkWeatherService::begin(const Configuration &configuration) {
  mutex_ = xSemaphoreCreateMutex();
  commands_ = xQueueCreate(4, sizeof(Command));
  if (!mutex_ || !commands_) return false;
  if (xTaskCreatePinnedToCore(taskEntry, "network", 12288, this, 1, &task_, 0) != pdPASS) return false;
  applyConfiguration(configuration);
  return true;
}

void NetworkWeatherService::taskEntry(void *context) {
  static_cast<NetworkWeatherService *>(context)->taskLoop();
}

void NetworkWeatherService::applyConfiguration(const Configuration &configuration) {
  if (!commands_) return;
  Command command{CommandType::Configure, configuration};
  xQueueSend(commands_, &command, 0);
}

void NetworkWeatherService::requestRefresh() {
  if (!commands_) return;
  Command command{}; command.type = CommandType::Refresh;
  xQueueSend(commands_, &command, 0);
}

void NetworkWeatherService::requestWifiScan() {
  if (!commands_) return;
  Command command{}; command.type = CommandType::Scan;
  if (xQueueSend(commands_, &command, 0) == pdTRUE) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    wifiScan_.scanning = true;
    wifiScan_.failed = false;
    ++wifiScan_.revision;
    xSemaphoreGive(mutex_);
  }
}

NetworkState NetworkWeatherService::networkState() const {
  if (!mutex_) return NetworkState::Unconfigured;
  xSemaphoreTake(mutex_, portMAX_DELAY); const auto value = state_; xSemaphoreGive(mutex_); return value;
}

WeatherSnapshot NetworkWeatherService::weather() const {
  WeatherSnapshot value;
  if (!mutex_) return value;
  xSemaphoreTake(mutex_, portMAX_DELAY); value = weather_; xSemaphoreGive(mutex_); return value;
}

WifiScanSnapshot NetworkWeatherService::wifiScan() const {
  WifiScanSnapshot value;
  if (!mutex_) return value;
  xSemaphoreTake(mutex_, portMAX_DELAY); value = wifiScan_; xSemaphoreGive(mutex_); return value;
}

bool NetworkWeatherService::takeLocationUpdate(Configuration &configuration) {
  if (!mutex_) return false;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  const bool pending = locationUpdatePending_;
  if (pending) {
    configuration = locationUpdate_;
    locationUpdatePending_ = false;
  }
  xSemaphoreGive(mutex_);
  return pending;
}

uint32_t NetworkWeatherService::failures() const {
  if (!mutex_) return 0;
  xSemaphoreTake(mutex_, portMAX_DELAY); const uint32_t value = failures_; xSemaphoreGive(mutex_); return value;
}

void NetworkWeatherService::service(uint32_t now) {
  xSemaphoreTake(mutex_, portMAX_DELAY);
  if (weather_.status == WeatherStatus::Valid && now - weather_.fetchedMs > cfg::kWeatherStaleMs)
    weather_.status = WeatherStatus::Stale;
  xSemaphoreGive(mutex_);
  if (connected() && due(now, nextRefresh_)) {
    nextRefresh_ = now + cfg::kWeatherIntervalMs; requestRefresh();
  }
}

void NetworkWeatherService::connectWifi(const Configuration &configuration) {
  if (!configuration.wifiEnabled || !configuration.ssid[0]) {
    WiFi.disconnect(true); setState(mutex_, state_, NetworkState::Unconfigured); return;
  }
  setState(mutex_, state_, NetworkState::Connecting);
  WiFi.mode(WIFI_STA);
  WiFi.begin(configuration.ssid, configuration.password);
  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 15000) vTaskDelay(pdMS_TO_TICKS(100));
  setState(mutex_, state_, WiFi.status() == WL_CONNECTED ? NetworkState::Connected : NetworkState::RetryWait);
}

void NetworkWeatherService::scanWifi() {
  WifiScanSnapshot result;
  result.scanning = true;
  xSemaphoreTake(mutex_, portMAX_DELAY);
  result.revision = wifiScan_.revision;
  xSemaphoreGive(mutex_);

  WiFi.mode(WIFI_STA);
  const int16_t found = WiFi.scanNetworks(false, false, false, 120);
  if (found < 0) {
    result.failed = true;
  } else {
    for (int16_t i = 0; i < found && result.count < cfg::kMaxWifiScanResults; ++i) {
      const String ssid = WiFi.SSID(i);
      if (!ssid.length()) continue;
      bool duplicate = false;
      for (uint8_t j = 0; j < result.count; ++j) {
        if (strncmp(result.networks[j].ssid, ssid.c_str(), sizeof(result.networks[j].ssid)) == 0) {
          duplicate = true;
          if (WiFi.RSSI(i) > result.networks[j].rssi) result.networks[j].rssi = WiFi.RSSI(i);
          break;
        }
      }
      if (duplicate) continue;
      WifiNetwork &network = result.networks[result.count++];
      strlcpy(network.ssid, ssid.c_str(), sizeof(network.ssid));
      network.rssi = WiFi.RSSI(i);
      network.secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }
  }
  WiFi.scanDelete();
  result.scanning = false;
  ++result.revision;
  xSemaphoreTake(mutex_, portMAX_DELAY); wifiScan_ = result; xSemaphoreGive(mutex_);
}

bool NetworkWeatherService::fetchIpLocation(Configuration &configuration) {
  if (WiFi.status() != WL_CONNECTED) return false;
  WiFiClientSecure client;
  client.setCACert(kIpLocationRootCa);
  client.setHandshakeTimeout(5);
  HTTPClient http;
  http.setConnectTimeout(5000); http.setTimeout(10000); http.setRedirectLimit(1);
  if (!http.begin(client, cfg::kIpLocationUrl)) {
    Serial.println("IP location failed: HTTPS setup");
    return false;
  }
  const int status = http.GET();
  const int length = http.getSize();
  if (status != HTTP_CODE_OK || length > 4096) {
    Serial.printf("IP location failed: HTTP %d\n", status);
    http.end();
    return false;
  }
  const String payload = http.getString();
  http.end();
  if (!payload.length() || payload.length() > 4096) {
    Serial.println("IP location failed: invalid body size");
    return false;
  }

  StaticJsonDocument<1536> doc;
  const DeserializationError error = deserializeJson(doc, payload, DeserializationOption::NestingLimit(4));
  if (error || !doc["success"].as<bool>()) {
    const char *message = doc["message"] | error.c_str();
    Serial.printf("IP location failed: %s\n", message ? message : "invalid response");
    return false;
  }

  const char *city = doc["city"] | "";
  const char *region = doc["region"] | "";
  const char *regionCode = doc["region_code"] | "";
  const char *countryCode = doc["country_code"] | "";
  const char *ianaTimezone = doc["timezone"]["id"] | "";
  const double latitude = doc["latitude"] | NAN;
  const double longitude = doc["longitude"] | NAN;
  const int32_t utcOffset = doc["timezone"]["offset"] | INT32_MAX;
  if (!city[0] || !isfinite(latitude) || !isfinite(longitude) || latitude < -90.0 || latitude > 90.0 ||
      longitude < -180.0 || longitude > 180.0 || !ianaTimezone[0] || utcOffset == INT32_MAX) {
    Serial.println("IP location failed: missing or invalid fields");
    return false;
  }

  Configuration resolved = configuration;
  if ((strcmp(countryCode, "US") == 0 || strcmp(countryCode, "CA") == 0) && regionCode[0])
    snprintf(resolved.location, sizeof(resolved.location), "%s, %s", city, regionCode);
  else if (countryCode[0])
    snprintf(resolved.location, sizeof(resolved.location), "%s, %s", city, countryCode);
  else if (region[0])
    snprintf(resolved.location, sizeof(resolved.location), "%s, %s", city, region);
  else
    strlcpy(resolved.location, city, sizeof(resolved.location));
  resolved.latitude = latitude;
  resolved.longitude = longitude;
  if (!timezoneRule(ianaTimezone, utcOffset, resolved.timezone, sizeof(resolved.timezone))) {
    Serial.println("IP location failed: unsupported UTC offset");
    return false;
  }

  const bool changed = locationChanged(configuration, resolved);
  configuration = resolved;
  Serial.printf("IP location: %s (%.5f, %.5f), %s -> %s\n", configuration.location,
                configuration.latitude, configuration.longitude, ianaTimezone, configuration.timezone);
  if (changed) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    locationUpdate_ = configuration;
    locationUpdatePending_ = true;
    xSemaphoreGive(mutex_);
  }
  return true;
}

bool NetworkWeatherService::parseIsoUtc(const char *value, time_t &result) {
  if (!value || strlen(value) < 16) return false;
  int year = 0, month = 0, day = 0, hour = 0, minute = 0;
  if (sscanf(value, "%d-%d-%dT%d:%d", &year, &month, &day, &hour, &minute) != 5 ||
      month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 ||
      minute < 0 || minute > 59) return false;
  const int64_t seconds = daysFromCivil(year, month, day) * 86400LL + hour * 3600LL + minute * 60LL;
  if (seconds <= 0 || seconds > INT32_MAX) return false;
  result = static_cast<time_t>(seconds);
  return true;
}

bool parseHttpDate(const char *value, time_t &result) {
  if (!value) return false;
  char weekday[4]{};
  char monthName[4]{};
  char zone[4]{};
  int day = 0, year = 0, hour = 0, minute = 0, second = 0;
  if (sscanf(value, "%3s, %d %3s %d %d:%d:%d %3s", weekday, &day, monthName,
             &year, &hour, &minute, &second, zone) != 8 || strcmp(zone, "GMT") != 0) return false;
  static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  unsigned month = 0;
  while (month < 12 && strcmp(monthName, months[month]) != 0) ++month;
  if (month == 12 || day < 1 || day > 31 || year < 2020 || year >= 2100 ||
      hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 60) return false;
  const int64_t seconds = daysFromCivil(year, month + 1, day) * 86400LL +
                          hour * 3600LL + minute * 60LL + min(second, 59);
  if (seconds <= 0 || seconds > INT32_MAX) return false;
  result = static_cast<time_t>(seconds);
  return true;
}

void NetworkWeatherService::fetchWeather(const Configuration &configuration) {
  if (WiFi.status() != WL_CONNECTED) return;
  xSemaphoreTake(mutex_, portMAX_DELAY); weather_.status = WeatherStatus::Fetching; xSemaphoreGive(mutex_);
  char url[384];
  snprintf(url, sizeof(url),
      "https://api.open-meteo.com/v1/forecast?latitude=%.5f&longitude=%.5f&current=temperature_2m,weather_code&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset&temperature_unit=celsius&timezone=UTC&timeformat=unixtime&forecast_days=1",
      configuration.latitude, configuration.longitude);
  WiFiClientSecure client;
  client.setCACert(kRootCa);
  client.setHandshakeTimeout(5);
  HTTPClient http;
  http.setConnectTimeout(5000); http.setTimeout(15000); http.setRedirectLimit(1);
  const char *headers[] = {"Date"};
  http.collectHeaders(headers, 1);
  WeatherSnapshot candidate = weather();
  candidate.configRevision = configuration.revision;
  strlcpy(candidate.location, configuration.location, sizeof(candidate.location));
  candidate.serverUtc = 0;
  if (!http.begin(client, url)) { strlcpy(candidate.error, "HTTPS setup failed", sizeof(candidate.error)); goto fail; }
  {
    const int status = http.GET();
    const int length = http.getSize();
    if (status != HTTP_CODE_OK || length > 8192) {
      if (status < 0) {
        const String reason = HTTPClient::errorToString(status);
        snprintf(candidate.error, sizeof(candidate.error), "Network: %s", reason.c_str());
      } else {
        snprintf(candidate.error, sizeof(candidate.error), "HTTP %d", status);
      }
      http.end(); goto fail;
    }
    parseHttpDate(http.header("Date").c_str(), candidate.serverUtc);
    const String payload = http.getString();
    http.end();
    if (!payload.length()) {
      strlcpy(candidate.error, "Weather body empty", sizeof(candidate.error)); goto fail;
    }
    if (payload.length() > 8192) {
      snprintf(candidate.error, sizeof(candidate.error), "Weather body %u B", static_cast<unsigned>(payload.length()));
      goto fail;
    }
    StaticJsonDocument<4096> doc;
    const DeserializationError error = deserializeJson(doc, payload, DeserializationOption::NestingLimit(8));
    if (error) {
      snprintf(candidate.error, sizeof(candidate.error), "JSON: %s (%u B)", error.c_str(),
               static_cast<unsigned>(payload.length()));
      Serial.printf("Weather JSON error: %s, bytes=%u, body=%s\n", error.c_str(),
                    static_cast<unsigned>(payload.length()), payload.c_str());
      goto fail;
    }
    JsonVariantConst currentTemperature = doc["current"]["temperature_2m"];
    JsonVariantConst currentCode = doc["current"]["weather_code"];
    JsonVariantConst dailyHigh = doc["daily"]["temperature_2m_max"][0];
    JsonVariantConst dailyLow = doc["daily"]["temperature_2m_min"][0];
    JsonVariantConst currentTime = doc["current"]["time"];
    JsonVariantConst sunrise = doc["daily"]["sunrise"][0];
    JsonVariantConst sunset = doc["daily"]["sunset"][0];
    if (currentTemperature.isNull()) { strlcpy(candidate.error, "Missing current temp", sizeof(candidate.error)); goto fail; }
    if (currentCode.isNull()) { strlcpy(candidate.error, "Missing weather code", sizeof(candidate.error)); goto fail; }
    if (dailyHigh.isNull()) { strlcpy(candidate.error, "Missing daily high", sizeof(candidate.error)); goto fail; }
    if (dailyLow.isNull()) { strlcpy(candidate.error, "Missing daily low", sizeof(candidate.error)); goto fail; }
    if (currentTime.isNull()) { strlcpy(candidate.error, "Missing current time", sizeof(candidate.error)); goto fail; }
    const float currentC = currentTemperature.as<float>();
    const float highC = dailyHigh.as<float>();
    const float lowC = dailyLow.as<float>();
    const int weatherCode = currentCode.as<int>();
    const int64_t observation = currentTime.as<int64_t>();
    if (!isfinite(currentC) || !isfinite(highC) || !isfinite(lowC) ||
        currentC < -100.0F || currentC > 70.0F || highC < -100.0F || highC > 70.0F ||
        lowC < -100.0F || lowC > 70.0F) {
      strlcpy(candidate.error, "Invalid temperatures", sizeof(candidate.error)); goto fail;
    }
    if (weatherCode < 0 || weatherCode > 99) { strlcpy(candidate.error, "Invalid weather code", sizeof(candidate.error)); goto fail; }
    if (observation < 1577836800LL || observation >= 4102444800LL) {
      strlcpy(candidate.error, "Invalid weather time", sizeof(candidate.error)); goto fail;
    }
    candidate.currentC = currentC;
    candidate.highC = highC;
    candidate.lowC = lowC;
    candidate.weatherCode = weatherCode;
    candidate.observationUtc = static_cast<time_t>(observation);
    if (!candidate.serverUtc) candidate.serverUtc = candidate.observationUtc;
    if (!sunrise.isNull()) candidate.sunriseUtc = static_cast<time_t>(sunrise.as<int64_t>());
    if (!sunset.isNull()) candidate.sunsetUtc = static_cast<time_t>(sunset.as<int64_t>());
    candidate.fetchedMs = millis(); candidate.status = WeatherStatus::Valid; candidate.error[0] = '\0';
    ++candidate.revision;
    xSemaphoreTake(mutex_, portMAX_DELAY); weather_ = candidate; xSemaphoreGive(mutex_); return;
  }
fail:
  candidate.status = candidate.fetchedMs ? WeatherStatus::Stale : WeatherStatus::Fault;
  ++candidate.revision;
  Serial.printf("Weather fetch failed: %s\n", candidate.error);
  xSemaphoreTake(mutex_, portMAX_DELAY); ++failures_; weather_ = candidate; xSemaphoreGive(mutex_);
}

void NetworkWeatherService::taskLoop() {
  Configuration active{}; bool configured = false;
  for (;;) {
    Command command{};
    if (xQueueReceive(commands_, &command, pdMS_TO_TICKS(500)) == pdTRUE) {
      if (command.type == CommandType::Configure) {
        active = command.configuration; configured = true; connectWifi(active);
        if (networkState() == NetworkState::Connected) {
          fetchIpLocation(active);
          fetchWeather(active);
        }
      } else if (command.type == CommandType::Refresh) {
        if (configured && networkState() == NetworkState::Connected) fetchWeather(active);
      } else {
        scanWifi();
      }
    }
    if (configured && WiFi.status() != WL_CONNECTED && active.wifiEnabled && active.ssid[0]) {
      const uint32_t now = millis();
      if (due(now, nextRetry_)) {
        static const uint16_t delays[] = {5, 15, 30, 60, 300};
        connectWifi(active);
        if (networkState() == NetworkState::Connected) {
          fetchIpLocation(active);
          fetchWeather(active);
        }
        nextRetry_ = now + delays[min<uint8_t>(retryIndex_, 4)] * 1000UL;
        retryIndex_ = min<uint8_t>(retryIndex_ + 1, 4);
      }
    } else if (WiFi.status() == WL_CONNECTED) retryIndex_ = 0;
  }
}
} // namespace dw