#include "HolidayService.h"
#include <cstring>

namespace dw {
namespace {
bool leap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }
int daysInMonth(int y, int m) {
  static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  return m == 2 && leap(y) ? 29 : days[m - 1];
}
int weekday(int y, int m, int d) {
  if (m < 3) { m += 12; --y; }
  const int k = y % 100, j = y / 100;
  const int h = (d + 13 * (m + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
  return (h + 6) % 7;
}
int nthWeekday(int y, int m, int wanted, int nth) {
  return 1 + ((7 + wanted - weekday(y, m, 1)) % 7) + 7 * (nth - 1);
}
int lastWeekday(int y, int m, int wanted) {
  const int last = daysInMonth(y, m);
  return last - ((7 + weekday(y, m, last) - wanted) % 7);
}
void setLabel(char *out, size_t cap, const char *value, bool observed) {
  snprintf(out, cap, observed ? "%s (Observed)" : "%s", value);
}
bool fixedHoliday(int y, int m, int d, int hm, int hd, const char *name,
                  char *out, size_t cap) {
  if (m == hm && d == hd) { setLabel(out, cap, name, false); return true; }
  const int wd = weekday(y, hm, hd);
  if (wd == 6 && ((m == hm && d == hd - 1) || (hm == 1 && hd == 1 && m == 12 && d == 31))) {
    setLabel(out, cap, name, true); return true;
  }
  if (wd == 0 && m == hm && d == hd + 1) { setLabel(out, cap, name, true); return true; }
  return false;
}
}

void HolidayService::labelForDate(const tm &date, char *label, size_t capacity) {
  if (!label || !capacity) return;
  label[0] = '\0';
  const int y = date.tm_year + 1900, m = date.tm_mon + 1, d = date.tm_mday;
  if (fixedHoliday(y,m,d,1,1,"New Year's Day",label,capacity) ||
      fixedHoliday(y,m,d,6,19,"Juneteenth",label,capacity) ||
      fixedHoliday(y,m,d,7,4,"Independence Day",label,capacity) ||
      fixedHoliday(y,m,d,11,11,"Veterans Day",label,capacity) ||
      fixedHoliday(y,m,d,12,25,"Christmas Day",label,capacity)) return;
  if (m == 1 && d == nthWeekday(y,m,1,3)) setLabel(label,capacity,"Martin Luther King Jr. Day",false);
  else if (m == 2 && d == nthWeekday(y,m,1,3)) setLabel(label,capacity,"Washington's Birthday",false);
  else if (m == 5 && d == lastWeekday(y,m,1)) setLabel(label,capacity,"Memorial Day",false);
  else if (m == 9 && d == nthWeekday(y,m,1,1)) setLabel(label,capacity,"Labor Day",false);
  else if (m == 10 && d == nthWeekday(y,m,1,2)) setLabel(label,capacity,"Columbus Day",false);
  else if (m == 11 && d == nthWeekday(y,m,4,4)) setLabel(label,capacity,"Thanksgiving Day",false);
}
} // namespace dw