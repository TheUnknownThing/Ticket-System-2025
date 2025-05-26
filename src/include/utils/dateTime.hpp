#ifndef DATETIME_HPP
#define DATETIME_HPP

#include "utils/dateFormatter.hpp"
#include "utils/string32.hpp"
#include <string>

class DateTime {
private:
  int date_mmdd;      // -1 if not set
  int time_minutes;   // -1 if not set

public:
  // Default constructor
  DateTime() : date_mmdd(-1), time_minutes(-1) {}
  
  // Constructor with date only (mm-dd)
  DateTime(const sjtu::string32& date) : time_minutes(-1) {
    date_mmdd = parseDateToMMDD(date);
  }
  
  // Constructor with date only (std::string)
  DateTime(const std::string& date) : time_minutes(-1) {
    date_mmdd = parseDateToMMDDStd(date);
  }
  
  // Constructor with time only (hh:mm)
  DateTime(const sjtu::string32& time, bool is_time) : date_mmdd(-1) {
    if (is_time) {
      time_minutes = parseTimeToMinutes(time);
    } else {
      date_mmdd = parseDateToMMDD(time);
      time_minutes = -1;
    }
  }
  
  // Constructor with both date and time
  DateTime(const sjtu::string32& date, const sjtu::string32& time) {
    date_mmdd = parseDateToMMDD(date);
    time_minutes = parseTimeToMinutes(time);
  }
  
  // Constructor with both date and time (std::string)
  DateTime(const std::string& date, const std::string& time) {
    date_mmdd = parseDateToMMDDStd(date);
    time_minutes = parseTimeToMinutes(sjtu::string32(time.c_str()));
  }
  
  // Setters
  void setDate(const sjtu::string32& date) {
    date_mmdd = parseDateToMMDD(date);
  }
  
  void setDate(const std::string& date) {
    date_mmdd = parseDateToMMDDStd(date);
  }
  
  void setTime(const sjtu::string32& time) {
    time_minutes = parseTimeToMinutes(time);
  }
  
  void setTime(const std::string& time) {
    time_minutes = parseTimeToMinutes(sjtu::string32(time.c_str()));
  }
  
  // Getters
  bool hasDate() const { return date_mmdd != -1; }
  bool hasTime() const { return time_minutes != -1; }
  
  int getDateMMDD() const { return date_mmdd; }
  int getTimeMinutes() const { return time_minutes; }
  
  std::string getDateString() const {
    return formatDateFromMMDD(date_mmdd);
  }
  
  std::string getTimeString() const {
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
      if (time_minutes >= 1440) {
        time_minutes %= 1440;
      }
    }
  }
  
  // Check if valid
  bool isValid() const {
    return hasDate() || hasTime();
  }
};

#endif // DATETIME_HPP
