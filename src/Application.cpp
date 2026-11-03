#include "Application.h"
#include <esp_system.h>

namespace dw {
void Application::setup() {
  Serial.begin(115200);
  const uint32_t serialStart = millis();
  while (!Serial && millis() - serialStart < 1500) delay(10);
  Serial.printf("Desk Widget %s reset=%d\n", DESK_WIDGET_VERSION, esp_reset_reason());

  if (!board_.begin()) {
    Serial.println("FATAL: display/PSRAM initialization failed");
    return;
  }
  renderer_.begin(board_); renderer_.showStartup("Loading configuration...");
  storage_.begin();
  if (!storage_.loadConfiguration(configuration_)) {
    configuration_ = Configuration{};
    Serial.println(storage_.available() ? "No valid configuration; defaults active" : "EEPROM unavailable; RAM defaults active");
  }
  renderer_.showStartup("Starting time and sensors...");
  time_.begin(configuration_);
  environment_.begin(storage_, time_.snapshot().valid, time_.utcNow());
  input_.begin(board_);
  ui_.begin(configuration_);
  network_.begin(configuration_);
  nextHealth_ = millis();
}

void Application::updateHealth(uint32_t now) {
  if (!due(now, nextHealth_)) return;
  nextHealth_ = now + 1000;
  health_.uptimeSeconds = now / 1000;
  health_.freeHeap = ESP.getFreeHeap();
  health_.freePsram = ESP.getFreePsram();
  health_.inputQueueOverflows = input_.overflows();
  health_.sgpDeadlineMisses = environment_.deadlineMisses();
  health_.sensorFailures = environment_.failures();
  health_.weatherFailures = network_.failures();
  if (ui_.screen() == Screen::Diagnostics) {
    InputEvent refresh;
    refresh.type = InputType::Rotation;
    refresh.atMs = now;
    ui_.handle(refresh);
  }
}

void Application::renderIfNeeded(uint32_t now) {
  const auto &t = time_.snapshot(); const auto &e = environment_.snapshot();
  const WeatherSnapshot w = network_.weather(); const NetworkState n = network_.networkState();
  const WifiScanSnapshot scan = network_.wifiScan();
  ui_.updateWifiScan(scan);
  const bool changed = ui_.dirty() || t.revision != renderedTimeRevision_ ||
      e.revision != renderedEnvironmentRevision_ || w.revision != renderedWeatherRevision_ ||
      n != renderedNetwork_ || scan.revision != renderedWifiScanRevision_;
  if (!changed || !due(now, nextRender_)) return;
  renderer_.render(ui_, configuration_, t, e, w, n, health_);
  nextRender_ = now + cfg::kDisplayMinFrameIntervalMs;
  ui_.clearDirty(); renderedTimeRevision_ = t.revision; renderedEnvironmentRevision_ = e.revision;
  renderedWeatherRevision_ = w.revision; renderedNetwork_ = n;
  renderedWifiScanRevision_ = scan.revision;
}

void Application::loop() {
  const uint32_t now = millis();
  input_.service(now);
  InputEvent event;
  while (input_.pop(event)) ui_.handle(event);
  if (ui_.wifiScanRequested()) network_.requestWifiScan();
  network_.service(now);
  Configuration located;
  if (network_.takeLocationUpdate(located)) {
    located.revision = configuration_.revision + 1;
    configuration_ = located;
    if (!storage_.saveConfiguration(configuration_)) ++health_.saveFailures;
    time_.applyConfiguration(configuration_);
    ui_.updateAutomaticLocation(configuration_);
  }
  const WeatherSnapshot weather = network_.weather();
  time_.service(now, network_.connected(), weather.serverUtc);
  environment_.service(now, time_.snapshot().valid, time_.utcNow());
  input_.setTvocTrend(environment_.snapshot().tvocTrend);
  if (ui_.saveRequested()) {
    Configuration next = ui_.draft(); next.revision = configuration_.revision + 1;
    const bool saved = storage_.saveConfiguration(next);
    if (saved) {
      configuration_ = next; time_.applyConfiguration(configuration_); network_.applyConfiguration(configuration_);
    } else ++health_.saveFailures;
    ui_.saveCompleted(saved, configuration_);
  }
  if (ui_.resetRequested()) {
    Configuration defaults{}; defaults.revision = configuration_.revision + 1;
    const bool saved = storage_.saveConfiguration(defaults);
    if (saved) { configuration_ = defaults; time_.applyConfiguration(configuration_); network_.applyConfiguration(configuration_); }
    else ++health_.saveFailures;
    ui_.saveCompleted(saved, configuration_);
  }
  updateHealth(now); renderIfNeeded(now); delay(1);
}
} // namespace dw