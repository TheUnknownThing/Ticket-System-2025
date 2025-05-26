#ifndef DATETIME_HPP
#define DATETIME_HPP

#include "utils/dateFormatter.hpp"
#include "utils/string32.hpp"
#include <string>

class DateTime {
private:
  int date_mmdd;    // -1 if not set
  int time_minutes; // -1 if not set

public:
  // Default constructor
  DateTime() : date_mmdd(-1), time_minutes(-1) {}

  // Constructor with date only (sjtu::string32)
  DateTime(const sjtu::string32 &date) : time_minutes(-1) {
    date_mmdd = parseDateToMMDD(date);
  }

  // Constructor with date only (std::string)
  DateTime(const std::string &date) : time_minutes(-1) {
    date_mmdd = parseDateToMMDDStd(date);
  }

  // Constructor with time only
  DateTime(const sjtu::string32 &str, bool is_time) {
    if (is_time) {
      date_mmdd = -1;
      time_minutes = parseTimeToMinutes(str);
    } else {
      date_mmdd = parseDateToMMDD(str);
      time_minutes = -1;
    }
  }

  // Constructor with both date and time (sjtu::string32)
  DateTime(const sjtu::string32 &date, const sjtu::string32 &time) {
    date_mmdd = parseDateToMMDD(date);
    time_minutes = parseTimeToMinutes(time);
  }

  // Constructor with both date and time (std::string)
  DateTime(const std::string &date, const std::string &time) {
    date_mmdd = parseDateToMMDDStd(date);
    time_minutes = parseTimeToMinutes(sjtu::string32(time.c_str()));
  }

  // Constructor with MMDD int and minutes int
  DateTime(int mmdd, int minutes) : date_mmdd(mmdd), time_minutes(minutes) {}

  // Setters
  void setDate(const sjtu::string32 &date) {
    date_mmdd = parseDateToMMDD(date);
  }

  void setDate(const std::string &date) {
    date_mmdd = parseDateToMMDDStd(date);
  }

  void setDate(int mmdd) { date_mmdd = mmdd; }

  void setTime(const sjtu::string32 &time) {
    time_minutes = parseTimeToMinutes(time);
  }

  void setTime(const std::string &time) {
    time_minutes = parseTimeToMinutes(sjtu::string32(time.c_str()));
  }

  void setTime(int minutes) { time_minutes = minutes; }

  // Getters
  bool hasDate() const { return date_mmdd != -1; }
  bool hasTime() const { return time_minutes != -1; }

  int getDateMMDD() const { return date_mmdd; }
  int getTimeMinutes() const { return time_minutes; }

  std::string getDateString() const {
    if (!hasDate())
      return "xx-xx";
    return formatDateFromMMDD(date_mmdd);
  }

  std::string getTimeString() const {
    if (!hasTime())
      return "xx:xx";
    return formatMinutesToTime(time_minutes);
  }

  std::string toString() const {
    if (hasDate() && hasTime()) {
      return getDateString() + " " + getTimeString();
    } else if (hasDate()) {
      return getDateString();
    } else if (hasTime()) {
      return getTimeString();
    } else {
      return "xx-xx xx:xx";
    }
  }

  // Add duration in minutes
  void addDuration(int duration_minutes) {
    if (hasDate() && hasTime()) {
      addDurationToDateTime(date_mmdd, time_minutes, duration_minutes);
    } else if (hasTime()) {
      time_minutes += duration_minutes;
      if (time_minutes >= 0) {
        time_minutes %= 1440;
        time_minutes = (time_minutes % 1440 + 1440) % 1440;
      }
    }
  }

  bool isValid() const { return hasDate() || hasTime(); }
};

#endif // DATETIME_HPP