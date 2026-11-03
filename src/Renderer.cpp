#include "Renderer.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <cmath>
#include <cstdio>

namespace dw {
namespace {
constexpr uint16_t kBackground = 0x0841;
constexpr uint16_t kPanel = 0x1082;
constexpr uint16_t kAccent = 0x2E9F;
constexpr uint16_t kMuted = 0x9CF3;
constexpr uint16_t kWarning = 0xFD20;
constexpr uint16_t kFault = 0xF986;
constexpr uint16_t kSun = 0xFFE0;
constexpr uint16_t kMoon = 0xFFDE;
constexpr uint16_t kCloud = 0xCE79;
constexpr uint16_t kCloudDark = 0x8410;
constexpr uint16_t kRain = 0x4D7F;
constexpr uint16_t kSnow = 0xEFFF;
constexpr uint16_t kTrendImproving = 0x067F;
constexpr uint16_t kTrendSteady = 0x07E0;
constexpr uint16_t kTrendWorsening = 0xFC20;

const GFXfont *fontForSize(uint8_t size, bool bold) {
  switch (size) {
    case 1: return bold ? &FreeSansBold9pt7b : &FreeSans9pt7b;
    case 2: return bold ? &FreeSansBold12pt7b : &FreeSans12pt7b;
    case 3: return bold ? &FreeSansBold18pt7b : &FreeSans18pt7b;
    default: return bold ? &FreeSansBold24pt7b : &FreeSans24pt7b;
  }
}

void drawText(Arduino_GFX *gfx, int16_t x, int16_t y, const char *text,
              uint16_t color, uint8_t size, bool bold) {
  gfx->setFont(fontForSize(size, bold));
  gfx->setTextSize(1);
  gfx->setTextColor(color);

  // GFX custom fonts position the cursor on the baseline. Measure this
  // particular string so callers can continue specifying its top-left edge.
  int16_t boundsX = 0;
  int16_t boundsY = 0;
  uint16_t boundsWidth = 0;
  uint16_t boundsHeight = 0;
  gfx->getTextBounds(text, x, 0, &boundsX, &boundsY, &boundsWidth, &boundsHeight);
  gfx->setCursor(x, y - boundsY);
  gfx->print(text);
}

bool isNighttime(const WeatherSnapshot &weather, time_t utcNow) {
  if (utcNow <= 0 || weather.sunriseUtc <= 0 || weather.sunsetUtc <= weather.sunriseUtc) return false;

  // Reuse the latest solar interval in 24-hour steps if weather updates are
  // temporarily stale, preventing the display from becoming stuck in day or night mode.
  constexpr time_t kDaySeconds = 24 * 60 * 60;
  time_t sunrise = weather.sunriseUtc;
  time_t sunset = weather.sunsetUtc;
  while (utcNow < sunrise) {
    sunrise -= kDaySeconds;
    sunset -= kDaySeconds;
  }
  while (utcNow >= sunrise + kDaySeconds) {
    sunrise += kDaySeconds;
    sunset += kDaySeconds;
  }
  return utcNow >= sunset;
}

uint8_t moonPhase(time_t utcNow) {
  // Known new moon: 2000-01-06 18:14 UTC. Rounding to the nearest eighth
  // produces the familiar eight displayed lunar phases.
  constexpr double kReferenceNewMoonUtc = 947182440.0;
  constexpr double kSynodicMonthSeconds = 2551442.9;
  double age = std::fmod(static_cast<double>(utcNow) - kReferenceNewMoonUtc,
                         kSynodicMonthSeconds);
  if (age < 0.0) age += kSynodicMonthSeconds;
  return static_cast<uint8_t>(age * 8.0 / kSynodicMonthSeconds + 0.5) & 7U;
}
}

void Renderer::begin(BoardServices &board) { board_ = &board; gfx_ = board.display(); }

void Renderer::showStartup(const char *message) {
  if (!gfx_) return;
  gfx_->fillScreen(kBackground);
  value(28, 180, "Raccoon Works", WHITE, 3);
  value(28, 225, "Desk Widget", kAccent, 2);
  label(28, 275, message, kMuted, 1);
  board_->present();
}

void Renderer::label(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t size) {
  drawText(gfx_, x, y, text, color, size, false);
}
void Renderer::value(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t size) {
  drawText(gfx_, x, y, text, color, size, true);
}

void Renderer::drawWeatherIcon(int16_t x, int16_t y, const WeatherSnapshot &weather,
                               time_t utcNow) {
  const int code = weather.weatherCode;
  const bool nighttime = isNighttime(weather, utcNow);
  const auto drawSun = [&](int16_t cx, int16_t cy) {
    gfx_->fillCircle(cx, cy, 12, kSun);
    gfx_->drawLine(cx, cy - 21, cx, cy - 16, kSun);
    gfx_->drawLine(cx, cy + 16, cx, cy + 21, kSun);
    gfx_->drawLine(cx - 21, cy, cx - 16, cy, kSun);
    gfx_->drawLine(cx + 16, cy, cx + 21, cy, kSun);
    gfx_->drawLine(cx - 15, cy - 15, cx - 11, cy - 11, kSun);
    gfx_->drawLine(cx + 11, cy - 11, cx + 15, cy - 15, kSun);
    gfx_->drawLine(cx - 15, cy + 15, cx - 11, cy + 11, kSun);
    gfx_->drawLine(cx + 11, cy + 11, cx + 15, cy + 15, kSun);
  };
  const auto drawMoon = [&](int16_t cx, int16_t cy) {
    constexpr int16_t radius = 14;
    constexpr double kPi = 3.14159265358979323846;
    const uint8_t phase = moonPhase(utcNow);
    const double angle = static_cast<double>(phase) * kPi / 4.0;
    const double sunX = std::sin(angle);
    const double sunZ = -std::cos(angle);

    gfx_->fillCircle(cx, cy, radius, kPanel);
    for (int16_t py = -radius; py <= radius; ++py) {
      for (int16_t px = -radius; px <= radius; ++px) {
        const int32_t distanceSquared = static_cast<int32_t>(px) * px +
                                        static_cast<int32_t>(py) * py;
        if (distanceSquared > radius * radius) continue;
        const double surfaceZ = std::sqrt(static_cast<double>(radius * radius - distanceSquared));
        if (static_cast<double>(px) * sunX + surfaceZ * sunZ >= 0.0)
          gfx_->drawPixel(cx + px, cy + py, kMoon);
      }
    }
    gfx_->drawCircle(cx, cy, radius, phase == 0 ? kMuted : kMoon);
  };
  const auto drawCloud = [&](int16_t cx, int16_t cy, uint16_t color) {
    gfx_->fillCircle(cx - 14, cy + 2, 11, color);
    gfx_->fillCircle(cx, cy - 6, 16, color);
    gfx_->fillCircle(cx + 17, cy + 1, 12, color);
    gfx_->fillRoundRect(cx - 25, cy, 53, 17, 8, color);
  };
  const auto drawSnowflake = [&](int16_t cx, int16_t cy) {
    gfx_->drawLine(cx - 5, cy, cx + 5, cy, kSnow);
    gfx_->drawLine(cx, cy - 5, cx, cy + 5, kSnow);
    gfx_->drawLine(cx - 4, cy - 4, cx + 4, cy + 4, kSnow);
    gfx_->drawLine(cx + 4, cy - 4, cx - 4, cy + 4, kSnow);
  };

  if (code == 0) {
    if (nighttime) drawMoon(x, y);
    else drawSun(x, y);
    return;
  }
  if (code == 1 || code == 2) {
    if (nighttime) drawMoon(x - 12, y - 10);
    else drawSun(x - 12, y - 10);
    drawCloud(x + 4, y + 5, kCloud);
    return;
  }
  if (code == 45 || code == 48) {
    drawCloud(x, y - 7, kCloudDark);
    gfx_->drawLine(x - 25, y + 17, x + 24, y + 17, kCloud);
    gfx_->drawLine(x - 19, y + 24, x + 28, y + 24, kCloud);
    return;
  }
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) {
    drawCloud(x, y - 7, kCloudDark);
    gfx_->drawLine(x - 17, y + 14, x - 22, y + 25, kRain);
    gfx_->drawLine(x, y + 14, x - 5, y + 25, kRain);
    gfx_->drawLine(x + 17, y + 14, x + 12, y + 25, kRain);
    return;
  }
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) {
    drawCloud(x, y - 8, kCloudDark);
    drawSnowflake(x - 15, y + 21);
    drawSnowflake(x + 13, y + 21);
    return;
  }
  if (code >= 95) {
    drawCloud(x, y - 8, kCloudDark);
    gfx_->fillTriangle(x + 2, y + 8, x - 7, y + 25, x + 2, y + 22, kSun);
    gfx_->fillTriangle(x + 2, y + 20, x + 10, y + 17, x - 2, y + 34, kSun);
    return;
  }
  drawCloud(x, y, code == 3 ? kCloudDark : kCloud);
}

void Renderer::drawTvocTrendArrow(int16_t x, int16_t y, TvocTrend trend) {
  uint16_t color = WHITE;
  int16_t x2 = x + 16;
  int16_t y2 = y;
  if (trend == TvocTrend::Improving) {
    color = kTrendImproving; y2 = y + 8;
  } else if (trend == TvocTrend::Worsening) {
    color = kTrendWorsening; y2 = y - 8;
  } else if (trend == TvocTrend::Steady) {
    color = kTrendSteady;
  } else if (trend == TvocTrend::Collecting) {
    gfx_->fillCircle(x + 2, y, 2, WHITE);
    gfx_->fillCircle(x + 9, y, 2, WHITE);
    gfx_->fillCircle(x + 16, y, 2, WHITE);
    return;
  } else {
    return;
  }
  for (int offset = -1; offset <= 1; ++offset)
    gfx_->drawLine(x, y + offset, x2, y2 + offset, color);
  if (trend == TvocTrend::Improving) {
    gfx_->fillTriangle(x2, y2, x2 - 7, y2 - 1, x2 - 1, y2 - 7, color);
  } else if (trend == TvocTrend::Worsening) {
    gfx_->fillTriangle(x2, y2, x2 - 7, y2 + 1, x2 - 1, y2 + 7, color);
  } else {
    gfx_->fillTriangle(x2, y2, x2 - 7, y2 - 5, x2 - 7, y2 + 5, color);
  }
}

const char *Renderer::qualityText(Quality q) const {
  switch (q) {
    case Quality::Initializing: return "INIT";
    case Quality::Stale: return "STALE";
    case Quality::Fault: return "FAULT";
    case Quality::Unavailable: return "--";
    default: return "";
  }
}

void Renderer::formatMeasurement(char *out, size_t capacity, const Measurement<float> &m,
                                 TemperatureUnit unit, bool temperature) const {
  if (m.quality != Quality::Valid && m.quality != Quality::Initializing && m.quality != Quality::Stale) {
    strlcpy(out, "--", capacity); return;
  }
  const float number = temperature ? displayTemperature(m.value, unit) : m.value;
  if (temperature) {
    snprintf(out, capacity, "%.1f%c", number,
             unit == TemperatureUnit::Fahrenheit ? 'F' : 'C');
  } else {
    snprintf(out, capacity, "%.1f%%", number);
  }
}

const char *Renderer::conditionName(int code) const {
  if (code == 0) return "Clear";
  if (code <= 2) return "Partly cloudy";
  if (code == 3) return "Overcast";
  if (code == 45 || code == 48) return "Fog";
  if (code >= 51 && code <= 57) return "Drizzle";
  if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82)) return "Rain";
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return "Snow";
  if (code >= 95) return "Thunderstorm";
  return "Unknown";
}

void Renderer::drawMain(const Configuration &configuration, const TimeSnapshot &time,
                        const EnvironmentalSnapshot &environment, const WeatherSnapshot &weather,
                        NetworkState network) {
  gfx_->fillScreen(kBackground);
  gfx_->fillRoundRect(12, 12, 456, 100, 10, kPanel);
  char text[96];
  if (time.valid) {
    strftime(text, sizeof(text), "%I:%M:%S %p", &time.local);
    value(26, 25, text, WHITE, 4);
    strftime(text, sizeof(text), "%A, %B %d, %Y", &time.local);
    label(28, 76, text, kMuted, 1);
    if (time.holiday[0]) label(28, 94, time.holiday, kAccent, 1);
  } else value(26, 38, "TIME UNSET", kWarning, 3);
  label(380, 24, network == NetworkState::Connected ? "WiFi" : "OFFLINE",
        network == NetworkState::Connected ? kAccent : kWarning, 1);
  label(380, 42, time.networkSynchronized ? "NTP" :
        (time.networkTimeFallback ? "NET" : (time.rtcAvailable ? "RTC" : "NO RTC")), kMuted, 1);

  gfx_->fillRoundRect(12, 122, 456, 174, 10, kPanel);
  label(26, 136, "OUTDOOR", kAccent, 1);
  if (weather.status == WeatherStatus::Valid || weather.status == WeatherStatus::Stale) {
    snprintf(text, sizeof(text), "%.0f%c", displayTemperature(weather.currentC, configuration.units),
             configuration.units == TemperatureUnit::Fahrenheit ? 'F' : 'C');
    value(26, 160, text, WHITE, 5);
    drawWeatherIcon(230, 185, weather, time.valid ? time.utc : 0);
    value(275, 166, conditionName(weather.weatherCode), WHITE, 2);
    snprintf(text, sizeof(text), "High %.0f  Low %.0f",
             displayTemperature(weather.highC, configuration.units),
             displayTemperature(weather.lowC, configuration.units));
    label(28, 225, text, kMuted, 2);
    tm local{};
    if (weather.sunriseUtc) { localtime_r(&weather.sunriseUtc, &local); strftime(text,sizeof(text),"Sunrise %I:%M %p",&local); label(28,260,text,kMuted,1); }
    if (weather.sunsetUtc) { localtime_r(&weather.sunsetUtc, &local); strftime(text,sizeof(text),"Sunset %I:%M %p",&local); label(245,260,text,kMuted,1); }
    if (weather.status == WeatherStatus::Stale) label(390, 136, "STALE", kWarning, 1);
  } else {
    value(28, 175, "Weather unavailable", kWarning, 2);
    label(28, 210, weather.error[0] ? weather.error : "Configure Wi-Fi", kMuted, 1);
  }

  gfx_->fillRoundRect(12, 306, 456, 132, 10, kPanel);
  label(26, 320, "INDOOR", kAccent, 1);
  char temp[24], rh[24];
  formatMeasurement(temp, sizeof(temp), environment.temperatureC, configuration.units, true);
  formatMeasurement(rh, sizeof(rh), environment.humidityPercent, configuration.units, false);
  value(28, 344, temp, WHITE, 3); value(160, 344, rh, WHITE, 3);
  if (environment.tvocPpb.quality == Quality::Valid) {
    snprintf(text, sizeof(text), "TVOC %u ppb", environment.tvocPpb.value);
    value(28, 394, text, WHITE, 2);
    drawTvocTrendArrow(215, 407, environment.tvocTrend);
  } else {
    snprintf(text,sizeof(text),"TVOC %s",qualityText(environment.tvocPpb.quality));
    value(28,394,text,kWarning,2);
  }
  if (environment.eco2Ppm.quality == Quality::Valid) snprintf(text,sizeof(text),"eCO2 %u ppm",environment.eco2Ppm.value);
  else snprintf(text,sizeof(text),"eCO2 %s",qualityText(environment.eco2Ppm.quality));
  value(255, 394, text, environment.eco2Ppm.quality == Quality::Valid ? WHITE : kWarning, 2);
  label(18, 455, "Press Encoder for Menu", kMuted, 1);
}

void Renderer::drawSettings(const UiController &ui) {
  gfx_->fillScreen(kBackground); value(22, 20, "Settings", WHITE, 3);
  static const char *items[] = {"Temperature units", "Automatic location", "Time zone", "Wi-Fi",
                                "Diagnostics", "Restore defaults", "Return home"};
  for (int i = 0; i < 7; ++i) {
    const int y = 78 + i * 48;
    if (i == ui.selected()) gfx_->fillRoundRect(18, y - 9, 444, 38, 6, kAccent);
    value(32, y, items[i], i == ui.selected() ? BLACK : WHITE, 2);
  }
  label(20, 450, ui.message(), kWarning, 1);
}

void Renderer::drawInfoPage(const UiController &ui, const Configuration &configuration,
                            const HealthSnapshot &health, NetworkState network) {
  gfx_->fillScreen(kBackground);
  char text[96];
  switch (ui.screen()) {
    case Screen::Units:
      value(22,20,"Temperature Units",WHITE,3);
      for (int i=0;i<2;++i) {
        if(i==ui.selected()) gfx_->fillRoundRect(20,100+i*70,440,48,6,kAccent);
        value(40,112+i*70,i==0?"Celsius":"Fahrenheit",i==ui.selected()?BLACK:WHITE,2);
      } break;
    case Screen::Location:
      value(22,20,"Automatic Location",WHITE,3);
      label(24,72,"Detected from public IP",kMuted,1);
      value(24,105,configuration.location,WHITE,2);
      snprintf(text,sizeof(text),"Lat %.5f",configuration.latitude); value(24,160,text,WHITE,2);
      snprintf(text,sizeof(text),"Lon %.5f",configuration.longitude); value(24,205,text,WHITE,2);
      label(24,265,"Updates after Wi-Fi connects",kAccent,1);
      label(24,300,"VPNs may change the result",kMuted,1); break;
    case Screen::TimeZone:
      value(22,20,"Time Zone",WHITE,3); value(24,100,configuration.timezone,WHITE,1);
      label(24,135,"Set automatically from IP",kMuted,1); break;
    case Screen::Wifi:
      value(22,20,"Wi-Fi Setup",WHITE,3);
      if (ui.wifiMode() == WifiUiMode::Networks) {
        const WifiScanSnapshot &scan = ui.wifiScan();
        label(24,58,scan.scanning?"Scanning for networks...":"Select a network",kMuted,1);
        const int first = max(0, min(ui.selected() - 2, static_cast<int>(scan.count) - 4));
        for (int row = 0; row < 5; ++row) {
          const int index = first + row;
          if (index > scan.count) break;
          const int y = 92 + row * 58;
          if (index == ui.selected()) gfx_->fillRoundRect(18,y-10,444,44,6,kAccent);
          if (index == scan.count) {
            value(34,y,"Rescan",index==ui.selected()?BLACK:WHITE,2);
          } else {
            const WifiNetwork &entry = scan.networks[index];
            value(34,y,entry.ssid,index==ui.selected()?BLACK:WHITE,2);
            snprintf(text,sizeof(text),"%ld dBm  %s",static_cast<long>(entry.rssi),entry.secure?"LOCK":"OPEN");
            label(300,y+5,text,index==ui.selected()?BLACK:kMuted,1);
          }
        }
      } else {
        label(24,62,"Network",kMuted,1); value(24,80,ui.draft().ssid,WHITE,2);
        const size_t passwordLength = strnlen(ui.wifiPassword(), sizeof(ui.draft().password));
        char masked[65]; memset(masked,'*',passwordLength); masked[passwordLength]='\0';
        label(24,130,"Password",kMuted,1); value(24,150,masked[0]?masked:"(empty)",WHITE,2);
        snprintf(text,sizeof(text),"%u / 64 characters",static_cast<unsigned>(passwordLength)); label(24,185,text,kMuted,1);
        const int choice = ui.wifiChoice();
        if (choice < 95) {
          label(24,235,"Press to add",kMuted,1);
          char character[4]={' ',0,0,0}; character[0]=ui.wifiCharacter();
          value(205,220,character[0]==' '?"SPACE":character,kAccent,4);
        } else {
          value(120,225,choice==95?"Delete":(choice==96?"Save":"Cancel"),
                choice==96?kAccent:(choice==97?kWarning:WHITE),3);
        }
        label(24,310,"Rotate: character/action",kMuted,1);
        label(24,335,"Press: choose",kMuted,1);
        label(24,370,"B2: Save & Connect",kAccent,2);
      }
      break;
    case Screen::Diagnostics:
      value(22,20,"Diagnostics",WHITE,3);
      snprintf(text,sizeof(text),"Firmware %s",DESK_WIDGET_VERSION); value(24,75,text,WHITE,1);
      snprintf(text,sizeof(text),"Uptime %lu s",health.uptimeSeconds); value(24,105,text,WHITE,2);
      snprintf(text,sizeof(text),"Heap %lu  PSRAM %lu",health.freeHeap,health.freePsram); value(24,145,text,WHITE,1);
      snprintf(text,sizeof(text),"Sensor failures %lu",health.sensorFailures); value(24,180,text,WHITE,1);
      snprintf(text,sizeof(text),"Weather failures %lu",health.weatherFailures); value(24,210,text,WHITE,1);
      snprintf(text,sizeof(text),"SGP deadline misses %lu",health.sgpDeadlineMisses); value(24,240,text,WHITE,1);
      snprintf(text,sizeof(text),"Input queue overflow %lu",health.inputQueueOverflows); value(24,270,text,WHITE,1); break;
    case Screen::ResetConfirm:
      value(22,20,"Restore Defaults?",WHITE,3); value(24,100,"No",ui.selected()==0?kAccent:WHITE,3); value(24,170,"Yes",ui.selected()==1?kFault:WHITE,3);
      label(24,230,"SGP30 baseline is preserved",kMuted,1); break;
    default: break;
  }
  label(20, 430, ui.message(), kWarning, 1);
  if (ui.screen() == Screen::Wifi && ui.wifiMode() == WifiUiMode::Password)
    label(20, 455, "B1 Cancel          B2 Save & Connect", kMuted, 1);
  else
    label(20, 455, "B1 Cancel / Encoder Confirm", kMuted, 1);
}

void Renderer::render(const UiController &ui, const Configuration &configuration,
                      const TimeSnapshot &time, const EnvironmentalSnapshot &environment,
                      const WeatherSnapshot &weather, NetworkState network,
                      const HealthSnapshot &health) {
  if (!gfx_) return;
  if (ui.screen() == Screen::Main) drawMain(configuration,time,environment,weather,network);
  else if (ui.screen() == Screen::Settings) drawSettings(ui);
  else drawInfoPage(ui,configuration,health,network);
  board_->present();
}
} // namespace dw