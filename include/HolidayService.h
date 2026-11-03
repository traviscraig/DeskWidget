#pragma once

#include <Arduino.h>
#include <time.h>

namespace dw {
class HolidayService {
public:
  static void labelForDate(const tm &date, char *label, size_t capacity);
};
} // namespace dw