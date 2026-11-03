#include "UiController.h"
#include <cstring>

namespace dw {
namespace {
constexpr char kWifiCharacters[] =
    " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
constexpr int kWifiCharacterCount = sizeof(kWifiCharacters) - 1;
constexpr int kWifiDeleteChoice = kWifiCharacterCount;
constexpr int kWifiSaveChoice = kWifiCharacterCount + 1;
constexpr int kWifiCancelChoice = kWifiCharacterCount + 2;
}

void UiController::begin(const Configuration &configuration) { active_ = draft_ = configuration; dirty_ = true; }
void UiController::enter(Screen screen) {
  screen_ = screen; selected_ = 0; dirty_ = true; message_[0] = '\0';
  if (screen == Screen::Wifi) {
    wifiMode_ = WifiUiMode::Networks;
    wifiChoice_ = 0;
    wifiScanRequested_ = true;
    strlcpy(message_, "Scanning...", sizeof(message_));
  }
}

void UiController::updateWifiScan(const WifiScanSnapshot &scan) {
  if (scan.revision == wifiScan_.revision) return;
  wifiScan_ = scan;
  if (wifiMode_ == WifiUiMode::Networks) {
    selected_ = constrain(selected_, 0, static_cast<int>(wifiScan_.count));
    if (scan.scanning) strlcpy(message_, "Scanning...", sizeof(message_));
    else if (scan.failed) strlcpy(message_, "Scan failed", sizeof(message_));
    else if (!scan.count) strlcpy(message_, "No networks found", sizeof(message_));
    else message_[0] = '\0';
  }
  dirty_ = true;
}

bool UiController::wifiScanRequested() {
  const bool requested = wifiScanRequested_;
  wifiScanRequested_ = false;
  return requested;
}

char UiController::wifiCharacter() const {
  return wifiChoice_ < kWifiCharacterCount ? kWifiCharacters[wifiChoice_] : '\0';
}

void UiController::submitWifiPassword() {
  const size_t length = strnlen(draft_.password, sizeof(draft_.password));
  if (length < 8) {
    strlcpy(message_, "Password needs 8+ chars", sizeof(message_));
  } else {
    saveRequested_ = true;
    strlcpy(message_, "Saving...", sizeof(message_));
  }
  dirty_ = true;
}

void UiController::handleWifi(const InputEvent &event) {
  if (event.type == InputType::Menu) {
    if (wifiMode_ == WifiUiMode::Password) submitWifiPassword();
    return;
  }
  if (event.type == InputType::Back) {
    if (wifiMode_ == WifiUiMode::Password) {
      wifiMode_ = WifiUiMode::Networks; selected_ = 0; message_[0] = '\0'; dirty_ = true;
    } else {
      draft_ = active_; enter(Screen::Settings); strlcpy(message_, "Changes discarded", sizeof(message_));
    }
    return;
  }
  if (event.type == InputType::Rotation) {
    if (wifiMode_ == WifiUiMode::Networks) {
      selected_ = constrain(selected_ + event.delta, 0, static_cast<int>(wifiScan_.count));
    } else {
      wifiChoice_ = constrain(wifiChoice_ + event.delta, 0, kWifiCancelChoice);
    }
    dirty_ = true; return;
  }
  if (event.type != InputType::Select) return;
  if (wifiMode_ == WifiUiMode::Networks) {
    if (selected_ >= wifiScan_.count) {
      wifiScanRequested_ = true; strlcpy(message_, "Scanning...", sizeof(message_)); dirty_ = true; return;
    }
    const WifiNetwork &network = wifiScan_.networks[selected_];
    const bool sameNetwork = strcmp(network.ssid, active_.ssid) == 0;
    strlcpy(draft_.ssid, network.ssid, sizeof(draft_.ssid));
    draft_.wifiEnabled = true;
    if (!network.secure) {
      draft_.password[0] = '\0'; saveRequested_ = true;
      strlcpy(message_, "Saving...", sizeof(message_)); dirty_ = true; return;
    }
    if (!sameNetwork) draft_.password[0] = '\0';
    wifiMode_ = WifiUiMode::Password; wifiChoice_ = 0; message_[0] = '\0'; dirty_ = true;
    return;
  }

  const size_t length = strnlen(draft_.password, sizeof(draft_.password));
  if (wifiChoice_ < kWifiCharacterCount) {
    if (length < sizeof(draft_.password) - 1) {
      draft_.password[length] = kWifiCharacters[wifiChoice_];
      draft_.password[length + 1] = '\0';
    } else strlcpy(message_, "Password is full", sizeof(message_));
  } else if (wifiChoice_ == kWifiDeleteChoice) {
    if (length) draft_.password[length - 1] = '\0';
  } else if (wifiChoice_ == kWifiSaveChoice) {
    submitWifiPassword();
  } else {
    wifiMode_ = WifiUiMode::Networks; selected_ = 0; message_[0] = '\0';
  }
  dirty_ = true;
}

void UiController::handle(const InputEvent &event) {
  if (screen_ == Screen::Wifi) { handleWifi(event); return; }
  if (event.type == InputType::Menu) { enter(screen_ == Screen::Main ? Screen::Settings : Screen::Main); return; }
  if (event.type == InputType::Back) {
    if (screen_ == Screen::Main) return;
    if (screen_ == Screen::Settings) enter(Screen::Main);
    else { draft_ = active_; enter(Screen::Settings); strlcpy(message_, "Changes discarded", sizeof(message_)); }
    return;
  }
  if (event.type == InputType::Rotation) {
    if (screen_ == Screen::Settings) selected_ = constrain(selected_ + event.delta, 0, 6);
    else if (screen_ == Screen::Units) selected_ = constrain(selected_ + event.delta, 0, 1);
    else if (screen_ == Screen::ResetConfirm) selected_ = constrain(selected_ + event.delta, 0, 1);
    dirty_ = true; return;
  }
  if (event.type != InputType::Select) return;
  if (screen_ == Screen::Main) {
    enter(Screen::Settings);
  } else if (screen_ == Screen::Settings) {
    static const Screen screens[] = {Screen::Units, Screen::Location, Screen::TimeZone, Screen::Wifi,
                                     Screen::Diagnostics, Screen::ResetConfirm, Screen::Main};
    enter(screens[selected_]);
  } else if (screen_ == Screen::Units) {
    draft_.units = selected_ == 0 ? TemperatureUnit::Celsius : TemperatureUnit::Fahrenheit;
    saveRequested_ = true; strlcpy(message_, "Saving...", sizeof(message_)); dirty_ = true;
  } else if (screen_ == Screen::Location) {
    strlcpy(message_, "Updates after Wi-Fi connects", sizeof(message_)); dirty_ = true;
  } else if (screen_ == Screen::ResetConfirm) {
    if (selected_ == 1) resetRequested_ = true; else enter(Screen::Settings);
  } else if (screen_ == Screen::TimeZone) {
    strlcpy(message_, "Set from IP location", sizeof(message_)); dirty_ = true;
  } else if (screen_ == Screen::Diagnostics) enter(Screen::Settings);
}

bool UiController::saveRequested() { const bool value = saveRequested_; saveRequested_ = false; return value; }
bool UiController::resetRequested() { const bool value = resetRequested_; resetRequested_ = false; return value; }
void UiController::saveCompleted(bool success, const Configuration &active) {
  if (success) {
    active_ = draft_ = active;
    strlcpy(message_, screen_ == Screen::Wifi ? "Saved; connecting..." : "Saved", sizeof(message_));
    if (screen_ == Screen::Wifi) wifiMode_ = WifiUiMode::Networks;
  }
  else { draft_ = active_; strlcpy(message_, "Save failed", sizeof(message_)); }
  dirty_ = true;
}

void UiController::updateAutomaticLocation(const Configuration &active) {
  active_ = active;
  strlcpy(draft_.location, active.location, sizeof(draft_.location));
  draft_.latitude = active.latitude;
  draft_.longitude = active.longitude;
  strlcpy(draft_.timezone, active.timezone, sizeof(draft_.timezone));
  draft_.revision = active.revision;
  if (screen_ == Screen::Location || screen_ == Screen::TimeZone)
    strlcpy(message_, "Location updated from IP", sizeof(message_));
  dirty_ = true;
}
} // namespace dw