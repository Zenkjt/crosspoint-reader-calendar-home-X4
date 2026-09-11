#pragma once

#include <cstdint>
#include "CalendarData.h"

class CalendarService {
 public:
  static constexpr const char* URL =
      "https://raw.githubusercontent.com/Zenkjt/x4-calendar-tool/main/today.json";

  void begin();
  bool refresh();

  const CalendarData& data() const { return current; }

  std::string monthText() const;
  std::string dayText() const;
  std::string weekdayText() const;
  std::string lunarText() const;

 private:
  static constexpr const char* CACHE_PATH = "/.crosspoint/calendar.json";
  static constexpr size_t MAX_JSON_BYTES = 4096;
  static constexpr uint32_t FETCH_TIMEOUT_MS = 8000;

  CalendarData current;

  bool parseAndValidate(const std::string& json, CalendarData& out) const;
  bool loadCache();
  bool saveCache(const std::string& json) const;
  void setSafeDefault();
};

extern CalendarService CALENDAR_SERVICE;
