#ifndef DATE_FORMATTER_HPP
#define DATE_FORMATTER_HPP

#include "utils/string32.hpp"
#include <string>

using sjtu::string32;

const int DAYS_IN_MONTH[] = {0, 0, 0, 0, 0, 0, 30, 31, 31, 30};

// Helper to parse "hh:mm" into minutes from midnight
int parseTimeToMinutes(const string32 &time_s32) {
  std::string time_str = time_s32.toString();
  if (time_str.length() != 5 || time_str[2] != ':') {
    return -1;
  }
  try {
    return std::stoi(time_str.substr(0, 2)) * 60 +
           std::stoi(time_str.substr(3, 2));
  } catch (const std::exception &) {
    return -1; // Error in conversion
  }
}

// Helper to parse "mm-dd" into an integer (e.g., 601 for "06-01")
int parseDateToMMDD(const string32 &date_s32) {
  std::string date_str = date_s32.toString();
  if (date_str.length() != 5 || date_str[2] != '-') {
    return -1;
  }
  try {
    return std::stoi(date_str.substr(0, 2)) * 100 +
           std::stoi(date_str.substr(3, 2));
  } catch (const std::exception &) {
    return -1;
  }
}
int parseDateToMMDDStd(const std::string &date_str) {
  if (date_str.length() != 5 || date_str[2] != '-') {
    return -1;
  }
  try {
    return std::stoi(date_str.substr(0, 2)) * 100 +
           std::stoi(date_str.substr(3, 2));
  } catch (const std::exception &) {
    return -1;
  }
}

// Helper to format minutes from midnight into "hh:mm"
std::string formatMinutesToTime(int minutes_in_day) {
  if (minutes_in_day < 0)
    return "xx:xx";
  char buf[6];
  std::sprintf(buf, "%02d:%02d", minutes_in_day / 60, minutes_in_day % 60);
  return std::string(buf);
}

// Helper to format MMDD integer into "mm-dd"
std::string formatDateFromMMDD(int date_mmdd) {
  if (date_mmdd < 0)
    return "xx-xx";
  if (date_mmdd / 100 < 1 || date_mmdd / 100 > 12 || date_mmdd % 100 < 1 ||
      date_mmdd % 100 > 31)
    return "xx-xx";
  char buf[6];
  std::sprintf(buf, "%02d-%02d", date_mmdd / 100, date_mmdd % 100);
  return std::string(buf);
}

// Adds duration_minutes to a date (mmdd) and time (minutes_in_day)
void addDurationToDateTime(int &date_mmdd, int &time_minutes_in_day,
                           int duration_minutes) {
  if (duration_minutes < 0)
    return;

  time_minutes_in_day += duration_minutes;
  while (time_minutes_in_day >= 1440) { // 1440 minutes in a day
    time_minutes_in_day -= 1440;
    int month = date_mmdd / 100;
    int day = date_mmdd % 100;

    day++;
    if (month < 6 || month > 9 || DAYS_IN_MONTH[month] == 0) {
      date_mmdd = -1; // Mark as invalid
      return;
    }
    if (day > DAYS_IN_MONTH[month]) {
      day = 1;
      month++;
      if (month > 9) {
        date_mmdd = -1;
        return;
      }
    }
    date_mmdd = month * 100 + day;
  }
}

void minusDurationFromDateTime(int &date_mmdd, int &time_minutes_in_day,
                               int duration_minutes) {
  if (duration_minutes < 0)
    return;

  time_minutes_in_day -= duration_minutes;
  while (time_minutes_in_day < 0) {
    time_minutes_in_day += 1440; // 1440 minutes in a day
    int month = date_mmdd / 100;
    int day = date_mmdd % 100;

    day--;
    if (day < 1) {
      month--;
      if (month < 6 || month > 9 || DAYS_IN_MONTH[month] == 0) {
        date_mmdd = -1; // Mark as invalid
        return;
      }
      day = DAYS_IN_MONTH[month];
    }
    date_mmdd = month * 100 + day;
  }
}

int calcDateDuration(int date1_mmdd, int date2_mmdd) {
  if (date1_mmdd < 0 || date2_mmdd < 0)
    return -1;
  if (date1_mmdd / 100 < 1 || date1_mmdd / 100 > 12 || date1_mmdd % 100 < 1 ||
      date1_mmdd % 100 > 31)
    return -1;
  if (date2_mmdd / 100 < 1 || date2_mmdd / 100 > 12 || date2_mmdd % 100 < 1 ||
      date2_mmdd % 100 > 31)
    return -1;

  int month1 = date1_mmdd / 100;
  int day1 = date1_mmdd % 100;
  int month2 = date2_mmdd / 100;
  int day2 = date2_mmdd % 100;

  int ans = 0;
  if (month1 == month2) {
    ans = day2 - day1;
  } else {
    ans += DAYS_IN_MONTH[month1] - day1; // Days left in month1
    for (int m = month1 + 1; m < month2; ++m) {
      ans += DAYS_IN_MONTH[m]; // Full months in between
    }
    ans += day2; // Days in month2
  }
  return ans < 0 ? -ans : ans;
}

int calcMinutesDuration(int date1_mmdd, int time1_minutes, int date2_mmdd,
                        int time2_minutes) {
  if (date1_mmdd < 0 || date2_mmdd < 0)
    return -1;
  if (date1_mmdd / 100 < 1 || date1_mmdd / 100 > 12 || date1_mmdd % 100 < 1 ||
      date1_mmdd % 100 > 31)
    return -1;
  if (date2_mmdd / 100 < 1 || date2_mmdd / 100 > 12 || date2_mmdd % 100 < 1 ||
      date2_mmdd % 100 > 31)
    return -1;

  int days_duration = calcDateDuration(date1_mmdd, date2_mmdd);
  if (days_duration < 0)
    return -1;

  int result = days_duration * 1440 + (time2_minutes - time1_minutes);
  return result < 0 ? -result : result;
}

#endif // DATE_FORMATTER_HPP