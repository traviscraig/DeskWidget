#include "StorageService.h"
#include "AppConfig.h"
#include <cstring>

namespace dw {
namespace {
constexpr uint32_t kCommit = 0x434F4D54UL;
struct RecordHeader {
  uint32_t magic;
  uint16_t schema;
  uint16_t length;
  uint32_t sequence;
  uint32_t crc;
  uint32_t commit;
};

uint32_t crc32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xFFFFFFFFUL;
  while (length--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; ++i) crc = (crc >> 1) ^ (0xEDB88320UL & -(crc & 1));
  }
  return ~crc;
}
bool newer(uint32_t a, uint32_t b) { return static_cast<int32_t>(a - b) > 0; }

class Writer {
public:
  Writer(uint8_t *data, size_t capacity) : data_(data), capacity_(capacity) {}
  template <typename T> void put(const T &value) { putBytes(&value, sizeof(value)); }
  void putString(const char *value, size_t maximum) {
    const uint8_t length = min<size_t>(strnlen(value, maximum), 255);
    put(length); putBytes(value, length);
  }
  void putBytes(const void *value, size_t length) {
    if (!ok_ || used_ + length > capacity_) { ok_ = false; return; }
    memcpy(data_ + used_, value, length); used_ += length;
  }
  bool ok() const { return ok_; }
  uint16_t size() const { return used_; }
private:
  uint8_t *data_; size_t capacity_; size_t used_{0}; bool ok_{true};
};

class Reader {
public:
  Reader(const uint8_t *data, size_t length) : data_(data), length_(length) {}
  template <typename T> void get(T &value) { getBytes(&value, sizeof(value)); }
  void getString(char *value, size_t capacity) {
    uint8_t stored = 0; get(stored);
    if (!ok_ || !capacity || used_ + stored > length_) { ok_ = false; return; }
    const size_t copied = min<size_t>(stored, capacity - 1);
    memcpy(value, data_ + used_, copied); value[copied] = '\0'; used_ += stored;
  }
  void getBytes(void *value, size_t length) {
    if (!ok_ || used_ + length > length_) { ok_ = false; return; }
    memcpy(value, data_ + used_, length); used_ += length;
  }
  bool ok() const { return ok_; }
private:
  const uint8_t *data_; size_t length_; size_t used_{0}; bool ok_{true};
};

uint16_t encodeConfiguration(const Configuration &c, uint8_t *data, size_t capacity) {
  Writer w(data, capacity);
  w.put(c.schemaVersion); w.put(c.revision);
  const uint8_t units = static_cast<uint8_t>(c.units); w.put(units);
  w.putString(c.location, sizeof(c.location)); w.put(c.latitude); w.put(c.longitude);
  w.putString(c.timezone, sizeof(c.timezone)); w.put(c.brightness);
  const uint8_t enabled = c.wifiEnabled; w.put(enabled);
  w.putString(c.ssid, sizeof(c.ssid)); w.putString(c.password, sizeof(c.password));
  return w.ok() ? w.size() : 0;
}

bool decodeConfiguration(const uint8_t *data, uint16_t length, Configuration &c) {
  Reader r(data, length); uint8_t units = 0, enabled = 0;
  r.get(c.schemaVersion); r.get(c.revision); r.get(units);
  r.getString(c.location, sizeof(c.location)); r.get(c.latitude); r.get(c.longitude);
  r.getString(c.timezone, sizeof(c.timezone)); r.get(c.brightness); r.get(enabled);
  r.getString(c.ssid, sizeof(c.ssid)); r.getString(c.password, sizeof(c.password));
  c.units = units == 0 ? TemperatureUnit::Celsius : TemperatureUnit::Fahrenheit;
  c.wifiEnabled = enabled;
  return r.ok() && c.schemaVersion == 1 && isfinite(c.latitude) && isfinite(c.longitude) &&
         c.latitude >= -90 && c.latitude <= 90 && c.longitude >= -180 && c.longitude <= 180;
}
}

bool StorageService::begin() {
  Wire.beginTransmission(cfg::kEepromAddress);
  available_ = Wire.endTransmission() == 0;
  return available_;
}

bool StorageService::waitReady(uint32_t timeoutMs) {
  const uint32_t started = millis();
  do {
    Wire.beginTransmission(cfg::kEepromAddress);
    if (Wire.endTransmission() == 0) return true;
    delay(1);
  } while (millis() - started < timeoutMs);
  return false;
}

bool StorageService::readBytes(uint16_t address, uint8_t *data, size_t length) {
  while (length) {
    const uint8_t chunk = min<size_t>(length, 28);
    Wire.beginTransmission(cfg::kEepromAddress);
    Wire.write(address >> 8); Wire.write(address & 0xFF);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(cfg::kEepromAddress, chunk) != chunk) return false;
    for (uint8_t i = 0; i < chunk; ++i) *data++ = Wire.read();
    address += chunk; length -= chunk;
  }
  return true;
}

bool StorageService::writeBytes(uint16_t address, const uint8_t *data, size_t length) {
  while (length) {
    const uint8_t pageRemaining = 32 - (address % 32);
    const uint8_t chunk = min<size_t>(length, min<uint8_t>(pageRemaining, 28));
    Wire.beginTransmission(cfg::kEepromAddress);
    Wire.write(address >> 8); Wire.write(address & 0xFF); Wire.write(data, chunk);
    if (Wire.endTransmission() != 0 || !waitReady()) return false;
    address += chunk; data += chunk; length -= chunk;
    yield();
  }
  return true;
}

bool StorageService::readRecord(uint16_t slot, uint32_t magic, uint8_t *payload,
                                uint16_t capacity, uint16_t &length, uint32_t &sequence) {
  RecordHeader h{};
  if (!readBytes(slot, reinterpret_cast<uint8_t *>(&h), sizeof(h)) || h.magic != magic ||
      h.schema != 1 || h.commit != kCommit || h.length > capacity) return false;
  if (!readBytes(slot + sizeof(h), payload, h.length) || crc32(payload, h.length) != h.crc) return false;
  length = h.length; sequence = h.sequence; return true;
}

bool StorageService::writeRecord(uint16_t slot, uint16_t slotSize, uint32_t magic,
                                 const uint8_t *payload, uint16_t length, uint32_t sequence) {
  if (length + sizeof(RecordHeader) > slotSize) return false;
  RecordHeader h{magic, 1, length, sequence, crc32(payload, length), 0};
  if (!writeBytes(slot, reinterpret_cast<const uint8_t *>(&h), sizeof(h)) ||
      !writeBytes(slot + sizeof(h), payload, length)) return false;
  uint8_t verify[768];
  if (length > sizeof(verify) || !readBytes(slot + sizeof(h), verify, length) ||
      crc32(verify, length) != h.crc) return false;
  h.commit = kCommit;
  return writeBytes(slot + offsetof(RecordHeader, commit),
                    reinterpret_cast<const uint8_t *>(&h.commit), sizeof(h.commit));
}

bool StorageService::loadConfiguration(Configuration &configuration) {
  if (!available_) return false;
  uint8_t a[768], b[768]; uint16_t al=0, bl=0; uint32_t as=0, bs=0;
  const bool av = readRecord(cfg::kConfigSlotA, cfg::kConfigMagic, a, sizeof(a), al, as);
  const bool bv = readRecord(cfg::kConfigSlotB, cfg::kConfigMagic, b, sizeof(b), bl, bs);
  if (!av && !bv) return false;
  return decodeConfiguration((bv && (!av || newer(bs, as))) ? b : a,
                             (bv && (!av || newer(bs, as))) ? bl : al, configuration);
}

bool StorageService::saveConfiguration(const Configuration &configuration) {
  if (!available_) return false;
  uint8_t payload[768]; const uint16_t length = encodeConfiguration(configuration, payload, sizeof(payload));
  if (!length) return false;
  uint8_t scratch[768]; uint16_t l=0; uint32_t as=0, bs=0;
  const bool av = readRecord(cfg::kConfigSlotA, cfg::kConfigMagic, scratch, sizeof(scratch), l, as);
  const bool bv = readRecord(cfg::kConfigSlotB, cfg::kConfigMagic, scratch, sizeof(scratch), l, bs);
  const uint32_t sequence = (!av && !bv) ? 1 : (av && (!bv || newer(as, bs)) ? as : bs) + 1;
  const uint16_t slot = !av || (bv && newer(bs, as)) ? cfg::kConfigSlotA : cfg::kConfigSlotB;
  return writeRecord(slot, cfg::kConfigSlotSize, cfg::kConfigMagic, payload, length, sequence);
}

bool StorageService::loadBaseline(SgpBaselineRecord &baseline) {
  if (!available_) return false;
  uint8_t a[64], b[64]; uint16_t al=0, bl=0; uint32_t as=0, bs=0;
  const bool av = readRecord(cfg::kBaselineSlotA, cfg::kBaselineMagic, a, sizeof(a), al, as);
  const bool bv = readRecord(cfg::kBaselineSlotB, cfg::kBaselineMagic, b, sizeof(b), bl, bs);
  if (!av && !bv) return false;
  const uint8_t *data = bv && (!av || newer(bs, as)) ? b : a;
  const uint16_t len = bv && (!av || newer(bs, as)) ? bl : al;
  if (len != sizeof(baseline)) return false;
  memcpy(&baseline, data, sizeof(baseline)); return true;
}

bool StorageService::saveBaseline(const SgpBaselineRecord &baseline) {
  if (!available_) return false;
  uint8_t scratch[64]; uint16_t l=0; uint32_t as=0, bs=0;
  const bool av = readRecord(cfg::kBaselineSlotA, cfg::kBaselineMagic, scratch, sizeof(scratch), l, as);
  const bool bv = readRecord(cfg::kBaselineSlotB, cfg::kBaselineMagic, scratch, sizeof(scratch), l, bs);
  const uint32_t sequence = (!av && !bv) ? 1 : (av && (!bv || newer(as, bs)) ? as : bs) + 1;
  const uint16_t slot = !av || (bv && newer(bs, as)) ? cfg::kBaselineSlotA : cfg::kBaselineSlotB;
  return writeRecord(slot, cfg::kBaselineSlotSize, cfg::kBaselineMagic,
                     reinterpret_cast<const uint8_t *>(&baseline), sizeof(baseline), sequence);
}

} // namespace dw