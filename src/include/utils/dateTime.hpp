#ifndef DATETIME_HPP
#define DATETIME_HPP

#include "utils/dateFormatter.hpp"
#include "utils/string32.hpp"
#include <ostream>
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
  DateTime(int mmdd, int minutes = -1) : date_mmdd(mmdd), time_minutes(minutes) {}

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
    int current_date = hasDate() ? date_mmdd : 0;
    int current_time = hasTime() ? time_minutes : 0;
    
    addDurationToDateTime(current_date, current_time, duration_minutes);
    
    date_mmdd = current_date;
    time_minutes = current_time;
  }

  void minusDuration(int duration_minutes) {
    int current_date = hasDate() ? date_mmdd : 0;
    int current_time = hasTime() ? time_minutes : 0;
    
    minusDurationFromDateTime(current_date, current_time, duration_minutes);
    
    date_mmdd = current_date;
    time_minutes = current_time;
  }

  bool isValid() const { return hasDate() || hasTime(); }

  bool operator<(const DateTime &other) const {
    int this_date = hasDate() ? date_mmdd : 0;
    int other_date = other.hasDate() ? other.date_mmdd : 0;
    int this_time = hasTime() ? time_minutes : 0;
    int other_time = other.hasTime() ? other.time_minutes : 0;
    
    if (this_date != other_date)
      return this_date < other_date;
    return this_time < other_time;
  }

  bool operator>(const DateTime &other) const {
    int this_date = hasDate() ? date_mmdd : 0;
    int other_date = other.hasDate() ? other.date_mmdd : 0;
    int this_time = hasTime() ? time_minutes : 0;
    int other_time = other.hasTime() ? other.time_minutes : 0;
    
    if (this_date != other_date)
      return this_date > other_date;
    return this_time > other_time;
  }

  bool operator==(const DateTime &other) const {
    int this_date = hasDate() ? date_mmdd : 0;
    int other_date = other.hasDate() ? other.date_mmdd : 0;
    int this_time = hasTime() ? time_minutes : 0;
    int other_time = other.hasTime() ? other.time_minutes : 0;
    
    return this_date == other_date && this_time == other_time;
  }

  bool operator!=(const DateTime &other) const {
    return !(*this == other);
  }

  bool operator<=(const DateTime &other) const {
    return *this < other || *this == other;
  }

  bool operator>=(const DateTime &other) const {
    return *this > other || *this == other;
  }

  friend std::ostream &operator<<(std::ostream &os, const DateTime &dt) {
    return os << dt.toString();
  }

  DateTime operator+(const DateTime &other) const {
    DateTime result = *this;
    
    int result_date = result.hasDate() ? result.date_mmdd : 0;
    int other_date = other.hasDate() ? other.date_mmdd : 0;
    int result_time = result.hasTime() ? result.time_minutes : 0;
    int other_time = other.hasTime() ? other.time_minutes : 0;
    
    result_date += other_date;
    result_time += other_time;
    
    if (result_time >= 1440) {
      result_date += result_time / 1440;
      result_time %= 1440;
    }
    
    result.date_mmdd = result_date;
    result.time_minutes = result_time;
    
    return result;
  }

  DateTime operator-(const DateTime &other) const {
    DateTime result = *this;
    
    int result_date = result.hasDate() ? result.date_mmdd : 0;
    int other_date = other.hasDate() ? other.date_mmdd : 0;
    int result_time = result.hasTime() ? result.time_minutes : 0;
    int other_time = other.hasTime() ? other.time_minutes : 0;
    
    result_date -= other_date;
    result_time -= other_time;
    
    if (result_time < 0) {
      result_date -= 1;
      result_time += 1440;
    }
    
    result.date_mmdd = result_date;
    result.time_minutes = result_time;
    
    return result;
  }

  DateTime operator+=(const DateTime &other) {
    *this = *this + other;
    return *this;
  }

  DateTime operator-=(const DateTime &other) {
    *this = *this - other;
    return *this;
  }
};

#endif // DATETIME_HPP