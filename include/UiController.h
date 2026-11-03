#pragma once
#include "DeskWidgetTypes.h"

namespace dw {
class UiController {
public:
  void begin(const Configuration &configuration);
  void handle(const InputEvent &event);
  void updateWifiScan(const WifiScanSnapshot &scan);
  Screen screen() const { return screen_; }
  int selected() const { return selected_; }
  bool dirty() const { return dirty_; }
  void clearDirty() { dirty_ = false; }
  bool saveRequested();
  bool resetRequested();
  Configuration &draft() { return draft_; }
  const Configuration &draft() const { return draft_; }
  void saveCompleted(bool success, const Configuration &active);
  void updateAutomaticLocation(const Configuration &active);
  const char *message() const { return message_; }
  bool wifiScanRequested();
  WifiUiMode wifiMode() const { return wifiMode_; }
  const WifiScanSnapshot &wifiScan() const { return wifiScan_; }
  int wifiChoice() const { return wifiChoice_; }
  char wifiCharacter() const;
  const char *wifiPassword() const { return draft_.password; }
private:
  void enter(Screen screen);
  void handleWifi(const InputEvent &event);
  void submitWifiPassword();
  Screen screen_{Screen::Main};
  int selected_{0};
  bool dirty_{true}, saveRequested_{false}, resetRequested_{false};
  Configuration active_{}, draft_{};
  WifiScanSnapshot wifiScan_{};
  WifiUiMode wifiMode_{WifiUiMode::Networks};
  int wifiChoice_{0};
  bool wifiScanRequested_{false};
  char message_[48]{};
};
} // namespace dw