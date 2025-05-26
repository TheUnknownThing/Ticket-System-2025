#ifndef TRAIN_MANAGER_HPP
#define TRAIN_MANAGER_HPP

#include "services/orderManager.hpp"
#include "stl/vector.hpp"
#include "storage/bptStorage.hpp"
#include "storage/cachedFileOperation.hpp"
#include "storage/varLengthFileOperation.hpp"
#include "utils/dateFormatter.hpp"
#include "utils/dateTime.hpp"
#include "utils/splitString.hpp"
#include "utils/string32.hpp"
#include <ostream>
#include <sstream>
#include <string>

using sjtu::string32;
using sjtu::vector;

struct Station {
  bool isStart;
  bool isEnd;
  string32 name;
  int price;
  int arrivalTimeOffset;
  int leavingTimeOffset;
  int index; // 0-based index
};

class StationBucketManager {
private:
  CachedFileOperation<Station> stationBucket;

public:
  StationBucketManager() = delete;
  StationBucketManager(const std::string &stationFile)
      : stationBucket(stationFile + "_station") {}
  int addStations(vector<Station> &stations);
  bool deleteStations(int bucketID, int num);
  vector<Station> queryStations(int bucketID, int num);
};

class TicketBucketManager {
private:
  VarLengthIntArrayFileOperation ticketBucket;

public:
  TicketBucketManager() = delete;
  TicketBucketManager(const std::string &ticketFile)
      : ticketBucket(ticketFile + "_ticket") {}
  int addTickets(int num_days, int num_stations_per_day, int init_value);
  vector<int> queryTickets(int bucketID);
  vector<int> queryTickets(int bucketID, int offset, int num_elements);
  void updateTickets(int bucketID, const vector<int> &tickets);
  void updateTickets(int bucketID, int offset, int num_elements,
                     const vector<int> &tickets);
};

struct Train {
  string32 trainID;
  int stationNum;
  int stationBucketID;
  int seatNum;
  int ticketBucketID;
  DateTime saleStartDate; // Stores only date part
  DateTime saleEndDate;   // Stores only date part
  DateTime startTime;     // Stores only time part
  char type;
  bool isReleased;

  bool operator==(const Train &other) const { return trainID == other.trainID; }
};

class TrainManager {
  friend class OrderManager;

private:
  BPTStorage<string32, Train> trainDB;
  StationBucketManager stationBucketManager;
  TicketBucketManager ticketBucketManager;

public:
  TrainManager(const std::string &trainFile);

  int addTrain(const string32 &trainID, int stationNum_val, int seatNum_val,
               const std::string &stations_str,      // -s ("s1|s2|s3")
               const std::string &prices_str,        // -p ("p1|p2")
               const std::string &startTime_str,     // -x ("hh:mm")
               const std::string &travelTimes_str,   // -t ("t1|t2")
               const std::string &stopoverTimes_str, // -o ("st1" or "_")
               const std::string &saleDates_str,     // -d ("d1|d2")
               char trainType);                      // -y

  int deleteTrain(const string32 &trainID);
  int releaseTrain(const string32 &trainID);
  std::string queryTrain(const string32 &trainID,
                         const string32 &date_s32); // date_s32 is "mm-dd"

private:
  /**
   * @brief Buy tickets for a specific train.
   * @return A pair of integers: the total price and the departure date of
   * train's START station in MMDD format.
   */
  std::pair<int, int> buyTicket(const string32 &trainID,
                                const DateTime &departureDate, int num,
                                const string32 &from, const string32 &to);

  bool refundTicket(const string32 &trainID, const DateTime &departureDate,
                    int num, const int from_idx, const int to_idx);

  vector<int> queryLeftSeats(const string32 &trainID, DateTime date,
                             const int from_station_idx,
                             const int to_station_idx);
};

int StationBucketManager::addStations(vector<Station> &stations) {
  if (stations.empty())
    return -1;
  int bucketID = stationBucket.write(stations[0]);
  for (size_t i = 1; i < stations.size(); ++i) {
    stationBucket.write(stations[i]);
  }
  return bucketID;
}

bool StationBucketManager::deleteStations(int bucketID, int num) {
  for (int i = 0; i < num; ++i) {
    stationBucket.remove(bucketID + i * sizeof(Station));
  }
  return true;
}

vector<Station> StationBucketManager::queryStations(int bucketID, int num) {
  vector<Station> stations_vec;
  for (int i = 0; i < num; ++i) {
    Station station;
    stationBucket.read(station, bucketID + i * sizeof(Station));
    stations_vec.push_back(station);
  }
  return stations_vec;
}

int TicketBucketManager::addTickets(int num_days, int num_stations_per_day,
                                    int init_value) {
  return ticketBucket.write(init_value, num_days * num_stations_per_day);
}

vector<int> TicketBucketManager::queryTickets(int bucketID) {
  return ticketBucket.read(bucketID);
}
vector<int> TicketBucketManager::queryTickets(int bucketID, int offset,
                                              int num_elements) {
  return ticketBucket.read(bucketID, offset, num_elements);
}

void TicketBucketManager::updateTickets(int bucketID,
                                        const vector<int> &tickets) {
  return ticketBucket.update(bucketID, tickets);
}

void TicketBucketManager::updateTickets(int bucketID, int offset,
                                        int num_elements,
                                        const vector<int> &tickets) {
  return ticketBucket.update(bucketID, offset, num_elements, tickets);
}

TrainManager::TrainManager(const std::string &trainFile)
    : trainDB(trainFile + "_train", string32::string32_MAX()),
      stationBucketManager(trainFile), ticketBucketManager(trainFile) {}

int TrainManager::addTrain(const string32 &trainID, int stationNum_val,
                           int seatNum_val, const std::string &stations_str,
                           const std::string &prices_str,
                           const std::string &startTime_str,
                           const std::string &travelTimes_str,
                           const std::string &stopoverTimes_str,
                           const std::string &saleDates_str, char trainType) {
  if (!trainDB.find(trainID).empty()) {
    return -1; // Train ID already exists
  }

  vector<string32> stationNames = splitString(stations_str, '|');
  vector<int> prices = splitStringToInt(prices_str, '|');

  DateTime trainStartTime(sjtu::string32(startTime_str.c_str()),
                          true); // true for time
  if (!trainStartTime.hasTime())
    return -1;

  vector<int> travelTimesMinutes = splitStringToInt(travelTimes_str, '|');

  vector<int> stopoverTimesMinutes;
  if (stopoverTimes_str == "_" && stationNum_val > 2) {
    stopoverTimesMinutes.assign(stationNum_val - 2, 0);
  } else {
    stopoverTimesMinutes = splitStringToInt(stopoverTimes_str, '|');
  }

  vector<string32> saleDateParts = splitString(saleDates_str, '|');
  if (saleDateParts.size() != 2)
    return -1; // Invalid sale date format
  DateTime saleStartDt(saleDateParts[0]);
  DateTime saleEndDt(saleDateParts[1]);

  if (!saleStartDt.hasDate() || !saleEndDt.hasDate() ||
      saleStartDt.getDateMMDD() > saleEndDt.getDateMMDD()) {
    return -1; // Invalid sale dates
  }

  if (stationNames.size() != static_cast<size_t>(stationNum_val) ||
      prices.size() != static_cast<size_t>(stationNum_val - 1) ||
      travelTimesMinutes.size() != static_cast<size_t>(stationNum_val - 1) ||
      (stationNum_val > 2 && stopoverTimesMinutes.size() !=
                                 static_cast<size_t>(stationNum_val - 2)) ||
      (stationNum_val == 2 && !stopoverTimesMinutes.empty() &&
       stopoverTimes_str != "_")) {
    return -1;
  }
  if (stationNum_val == 2 && stopoverTimes_str != "_" &&
      !stopoverTimesMinutes.empty()) {
    return -1; // For 2 stations, stopover times should be empty if not "_"
  }

  vector<Station> stationData;
  int currentRelativeTime = 0;

  for (int i = 0; i < stationNum_val; ++i) {
    Station s;
    s.name = stationNames[i];
    s.index = i;

    if (i == 0) {
      s.isStart = true;
      s.isEnd = false;
      s.arrivalTimeOffset = -1;
      s.leavingTimeOffset = 0;
      s.price = 0;
      currentRelativeTime = 0;
    } else {
      s.isStart = false;
      currentRelativeTime += travelTimesMinutes[i - 1];
      s.arrivalTimeOffset = currentRelativeTime;
      s.price = prices[i - 1];
      if (i == stationNum_val - 1) { // End station
        s.isEnd = true;
        s.leavingTimeOffset = -1; // No departure from end
      } else {                    // Intermediate station
        s.isEnd = false;
        currentRelativeTime += stopoverTimesMinutes[i - 1];
        s.leavingTimeOffset = currentRelativeTime;
      }
    }
    stationData.push_back(s);
  }

  int station_bID = stationBucketManager.addStations(stationData);
  if (station_bID == -1) {
    return -1;
  }

  Train newTrain;
  newTrain.trainID = trainID;
  newTrain.stationNum = stationNum_val;
  newTrain.stationBucketID = station_bID;
  newTrain.seatNum = seatNum_val;
  newTrain.ticketBucketID = -1;
  newTrain.saleStartDate = saleStartDt;
  newTrain.saleEndDate = saleEndDt;
  newTrain.startTime = trainStartTime;
  newTrain.type = trainType;
  newTrain.isReleased = false;

  trainDB.insert(trainID, newTrain);
  return 0;
}

int TrainManager::deleteTrain(const string32 &trainID) {
  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    return -1;
  }
  Train trainToDelete = foundTrains[0];
  if (trainToDelete.isReleased) {
    return -1; // Cannot delete a released train
  }

  trainDB.remove(trainID, trainToDelete);
  stationBucketManager.deleteStations(trainToDelete.stationBucketID,
                                      trainToDelete.stationNum);
  return 0;
}

int TrainManager::releaseTrain(const string32 &trainID) {
  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    return -1; // Train not found
  }
  Train trainToRelease = foundTrains[0];
  if (trainToRelease.isReleased) {
    return -1; // Already released
  }

  Train oldTrainState = trainToRelease;

  trainToRelease.isReleased = true;
  int numSaleDays = calcDateDuration(trainToRelease.saleStartDate.getDateMMDD(),
                                     trainToRelease.saleEndDate.getDateMMDD()) +
                    1;

  int ticket_bID = ticketBucketManager.addTickets(
      numSaleDays, trainToRelease.stationNum, trainToRelease.seatNum);

  if (ticket_bID == -1)
    return -1;

  trainToRelease.ticketBucketID = ticket_bID;

  trainDB.remove(trainID, oldTrainState);
  trainDB.insert(trainID, trainToRelease);

  return 0;
}

std::string TrainManager::queryTrain(const string32 &trainID,
                                     const string32 &date_s32) {
  DateTime queryDate(date_s32); // Parse "mm-dd" string
  if (!queryDate.hasDate()) {
    return "-1"; // Invalid date format
  }

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    return "-1"; // Train not found
  }
  Train train = foundTrains[0];

  if (queryDate.getDateMMDD() < train.saleStartDate.getDateMMDD() ||
      queryDate.getDateMMDD() > train.saleEndDate.getDateMMDD()) {
    return "-1"; // Not on sale on this date
  }

  std::ostringstream oss;
  oss << train.trainID.toString() << " " << train.type << "\n";

  vector<Station> stations = stationBucketManager.queryStations(
      train.stationBucketID, train.stationNum);

  int cumulativePrice = 0;

  // Base departure DateTime: queryDate + train's startTime
  DateTime baseDepartureDateTime(queryDate.getDateMMDD(),
                                 train.startTime.getTimeMinutes());

  vector<int> dailyLeftSeats;
  if (train.isReleased) {
    int dayOffsetInTicketBucket =
        calcDateDuration(train.saleStartDate.getDateMMDD(),
                         queryDate.getDateMMDD()) *
        train.stationNum;
    dailyLeftSeats = ticketBucketManager.queryTickets(
        train.ticketBucketID, dayOffsetInTicketBucket,
        train.stationNum - 1); // FIX: Maybe needed a fix here
    dailyLeftSeats =
        queryLeftSeats(trainID, queryDate, 0, train.stationNum - 1);
  }

  for (int i = 0; i < train.stationNum; ++i) {
    const Station &s = stations[i];
    std::string arrivalTimeStr, leavingTimeStr;

    if (s.isStart) {
      arrivalTimeStr = "xx-xx xx:xx";
      DateTime leavingDateTime = baseDepartureDateTime; // No offset
      leavingTimeStr = leavingDateTime.toString();
      cumulativePrice = 0;
    } else {
      DateTime arrivalDateTime = baseDepartureDateTime;
      arrivalDateTime.addDuration(s.arrivalTimeOffset);
      arrivalTimeStr = arrivalDateTime.toString();
      cumulativePrice += s.price; // s.price is price from previous to current

      if (s.isEnd) {
        leavingTimeStr = "xx-xx xx:xx";
      } else {
        DateTime leavingDateTime = baseDepartureDateTime;
        leavingDateTime.addDuration(s.leavingTimeOffset);
        leavingTimeStr = leavingDateTime.toString();
      }
    }

    oss << s.name.toString() << " " << arrivalTimeStr << " -> "
        << leavingTimeStr << " " << cumulativePrice << " ";

    if (s.isEnd) {
      oss << "x";
    } else {
      if (train.isReleased) {
        oss << dailyLeftSeats[i];
      } else {
        oss << train.seatNum;
      }
    }

    if (i < train.stationNum - 1) {
      oss << "\n";
    }
  }
  return oss.str();
}

std::pair<int, int> TrainManager::buyTicket(const string32 &trainID,
                                            const DateTime &departureDate,
                                            int num,
                                            const string32 &from_station_name,
                                            const string32 &to_station_name) {
  // 1. Find train
  // 2. Validate date, from, to stations (get their indices)
  // 3. Check seat availability using queryLeftSeats for all segments involved
  // 4. If available, update ticket counts in TicketBucketManager
  // 5. Calculate price
  // 6. Return {total_price, departureDate.getDateMMDD()}

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty() || !foundTrains[0].isReleased)
    return {-1, -1};
  Train train = foundTrains[0];
  // ... find from_idx, to_idx from names ...
  // ... check seats ...
  // ... update seats ...
  // ... calculate price ...
  return {-1, -1}; // Placeholder
}

bool TrainManager::refundTicket(const string32 &trainID,
                                const DateTime &departureDate, int num,
                                const int from_idx, const int to_idx) {
  // 1. Find train
  // 2. Validate
  // 3. Update ticket counts (increase) in TicketBucketManager

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty() || !foundTrains[0].isReleased)
    return false;
  Train train = foundTrains[0];
  // ... update seats ...
  return false; // Placeholder
}

vector<int> TrainManager::queryLeftSeats(const string32 &trainID, DateTime date,
                                         const int from_station_idx,
                                         const int to_station_idx) {
  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    return vector<int>(); // No such train
  }
  Train train = foundTrains[0];
  if (!train.isReleased) {
    vector<int> seats(to_station_idx - from_station_idx, train.seatNum);
    return seats;
  }

  int dayIndex =
      calcDateDuration(train.saleStartDate.getDateMMDD(), date.getDateMMDD());
  if (dayIndex < 0)
    return vector<int>(); // Date is before sale starts

  int baseOffsetForDay =
      dayIndex * (train.stationNum - 1); // FIX: Maybe needed a fix here
  int startOffsetInBucket = baseOffsetForDay + from_station_idx;

  int numElementsToQuery = to_station_idx - from_station_idx;
  if (numElementsToQuery <= 0)
    return vector<int>();

  return ticketBucketManager.queryTickets(
      train.ticketBucketID, startOffsetInBucket, numElementsToQuery);
}

#endif // TRAIN_MANAGER_HPP