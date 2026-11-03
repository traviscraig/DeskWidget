#pragma once

#include "BoardServices.h"
#include "EnvironmentalService.h"
#include "InputService.h"
#include "NetworkWeatherService.h"
#include "Renderer.h"
#include "StorageService.h"
#include "TimeService.h"
#include "UiController.h"

namespace dw {
class Application {
public:
  void setup();
  void loop();
private:
  void updateHealth(uint32_t now);
  void renderIfNeeded(uint32_t now);
  BoardServices board_;
  StorageService storage_;
  InputService input_;
  EnvironmentalService environment_;
  TimeService time_;
  NetworkWeatherService network_;
  UiController ui_;
  Renderer renderer_;
  Configuration configuration_;
  HealthSnapshot health_;
  uint32_t nextHealth_{0};
  uint32_t nextRender_{0};
  uint32_t renderedTimeRevision_{0}, renderedEnvironmentRevision_{0};
  uint32_t renderedWeatherRevision_{0};
  NetworkState renderedNetwork_{NetworkState::Unconfigured};
  uint32_t renderedWifiScanRevision_{0};
};
} // namespace dw