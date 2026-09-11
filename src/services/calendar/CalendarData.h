#pragma once

#include <string>

struct CalendarData {
  bool valid = false;
  std::string date;
  int day = 1;
  int month = 1;
  int weekday = 1;
  int lunarDay = 1;
  int lunarMonth = 1;
  std::string ownerName;
  std::string ownerPhone;
  std::string ownerEmail;
};
