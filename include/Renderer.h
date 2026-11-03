#pragma once

#include "BoardServices.h"
#include "DeskWidgetTypes.h"
#include "UiController.h"

namespace dw {
class Renderer {
public:
  void begin(BoardServices &board);
  void showStartup(const char *message);
  void render(const UiController &ui, const Configuration &configuration,
              const TimeSnapshot &time, const EnvironmentalSnapshot &environment,
              const WeatherSnapshot &weather, NetworkState network,
              const HealthSnapshot &health);
private:
  void drawMain(const Configuration &, const TimeSnapshot &, const EnvironmentalSnapshot &,
                const WeatherSnapshot &, NetworkState);
  void drawSettings(const UiController &);
  void drawInfoPage(const UiController &, const Configuration &, const HealthSnapshot &, NetworkState);
  void label(int16_t x, int16_t y, const char *text, uint16_t color = 0xBDF7, uint8_t size = 1);
  void value(int16_t x, int16_t y, const char *text, uint16_t color = WHITE, uint8_t size = 2);
  void drawWeatherIcon(int16_t x, int16_t y, const WeatherSnapshot &weather, time_t utcNow);
  void drawTvocTrendArrow(int16_t x, int16_t y, TvocTrend trend);
  const char *conditionName(int code) const;
  const char *qualityText(Quality quality) const;
  void formatMeasurement(char *out, size_t capacity, const Measurement<float> &measurement,
                         TemperatureUnit unit, bool temperature) const;
  Arduino_GFX *gfx_{nullptr};
  BoardServices *board_{nullptr};
};
} // namespace dw