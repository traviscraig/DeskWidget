#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "DeskWidgetTypes.h"

namespace dw {

struct SgpBaselineRecord {
  uint16_t eco2{0};
  uint16_t tvoc{0};
  uint16_t serial[3]{};
  time_t capturedUtc{0};
  uint32_t conditioningSeconds{0};
};

class StorageService {
public:
  bool begin();
  bool loadConfiguration(Configuration &configuration);
  bool saveConfiguration(const Configuration &configuration);
  bool loadBaseline(SgpBaselineRecord &baseline);
  bool saveBaseline(const SgpBaselineRecord &baseline);
  bool available() const { return available_; }

private:
  bool readBytes(uint16_t address, uint8_t *data, size_t length);
  bool writeBytes(uint16_t address, const uint8_t *data, size_t length);
  bool waitReady(uint32_t timeoutMs = 20);
  bool readRecord(uint16_t slot, uint32_t magic, uint8_t *payload,
                  uint16_t capacity, uint16_t &length, uint32_t &sequence);
  bool writeRecord(uint16_t slot, uint16_t slotSize, uint32_t magic,
                   const uint8_t *payload, uint16_t length, uint32_t sequence);
  bool available_{false};
};

} // namespace dw