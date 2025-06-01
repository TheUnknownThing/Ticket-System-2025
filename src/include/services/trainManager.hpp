#ifndef TRAIN_MANAGER_HPP
#define TRAIN_MANAGER_HPP

#include "stl/map.hpp"
#include "stl/vector.hpp"
#include "storage/bptStorage.hpp"
#include "storage/cachedFileOperation.hpp"
#include "storage/varLengthFileOperation.hpp"
#include "utils/dateFormatter.hpp"
#include "utils/dateTime.hpp"
#include "utils/logger.hpp"
#include "utils/splitString.hpp"
#include "utils/string32.hpp"
#include <climits>
#include <ostream>
#include <sstream>
#include <string>

using sjtu::map;
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
      : stationBucket(stationFile) {
    stationBucket.initialise();
    LOG("StationBucketManager initialized with file: " + stationFile);
  }
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
      : ticketBucket(ticketFile) {
    ticketBucket.initialise();
    LOG("TicketBucketManager initialized with file: " + ticketFile);
  }
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

  bool operator<=(const Train &other) const { return trainID <= other.trainID; }

  bool operator>(const Train &other) const { return trainID > other.trainID; }

  bool operator<(const Train &other) const { return trainID < other.trainID; }
};

struct CustomStringHasher {
  size_t operator()(const char *str) const {
    size_t hash = 5381;
    const size_t salt = 33;
    int c;

    while ((c = *str++)) {
      hash = ((hash << 5) + hash) + c + salt;
    }

    return hash;
  }
};

struct TicketCandidate {
  string32 trainID;
  int price;
  int duration;
  string32 fromStation;
  string32 toStation;
  DateTime departureDateTime;
  DateTime endDateTime;
  int seatNum;

  TicketCandidate(const string32 &id, int p, int dur, const string32 &from,
                  const string32 &to, const DateTime &dep, const DateTime &end,
                  int seat)
      : trainID(id), price(p), duration(dur), fromStation(from), toStation(to),
        departureDateTime(dep), endDateTime(end), seatNum(seat) {}

  TicketCandidate() = default;

  friend std::ostream &operator<<(std::ostream &os,
                                  const TicketCandidate &candidate) {
    os << candidate.trainID.toString() << " "
       << candidate.fromStation.toString() << " " << candidate.departureDateTime
       << " -> " << candidate.toStation.toString() << " "
       << candidate.endDateTime << " " << candidate.price << " "
       << candidate.seatNum;
    return os;
  }
};

class OrderManager;

class TrainManager {
  friend class OrderManager;

private:
  BPTStorage<string32, Train> trainDB;
  BPTStorage<std::pair<size_t, size_t>, string32, 500, 100>
      ticketLookupDB; // stationName, stationName -> trainID
  BPTStorage<size_t, string32> transferLookupDB; // fromStation -> trainID
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

  std::string queryTicket(const string32 &from, const string32 &to,
                          const string32 &date_s32,
                          const std::string &sortBy = "time");

  std::string queryTransfer(const string32 &from, const string32 &to,
                            const string32 &date_s32,
                            const std::string &sortBy = "time");

private:
  /**
   * @brief Buy tickets for a specific train.
   * @return A pair of integers: the total price and the departure date of
   * train's START station in MMDD format.
   */
  std::tuple<int, int, bool, int, int, int, int>
  buyTicket(const string32 &trainID, const DateTime &departureDate, int num,
            const string32 &from, const string32 &to);

  bool refundTicket(const string32 &trainID, const DateTime &departureDate,
                    int num, const int from_idx, const int to_idx);

  vector<int> queryLeftSeats(const string32 &trainID, DateTime date,
                             const int from_station_idx,
                             const int to_station_idx);

  bool updateLeftSeats(const string32 &trainID, DateTime date,
                       const int from_station_idx, const int to_station_idx,
                       int num);

  vector<TicketCandidate> querySingle(const string32 &from, const string32 &to,
                                      const DateTime &date,
                                      const std::string &sortBy = "time",
                                      bool isTransfer = false);
};

int StationBucketManager::addStations(vector<Station> &stations) {
  if (stations.empty()) {
    ERROR("addStations: empty stations vector");
    return -1;
  }
  int bucketID = stationBucket.write(stations[0]);
  for (size_t i = 1; i < stations.size(); ++i) {
    stationBucket.write(stations[i]);
  }
  LOG("Added " + std::to_string(stations.size()) +
      " stations with bucket ID: " + std::to_string(bucketID));
  return bucketID;
}

bool StationBucketManager::deleteStations(int bucketID, int num) {
  for (int i = 0; i < num; ++i) {
    stationBucket.remove(bucketID + i * sizeof(Station));
  }
  LOG("Deleted " + std::to_string(num) +
      " stations from bucket ID: " + std::to_string(bucketID));
  return true;
}

vector<Station> StationBucketManager::queryStations(int bucketID, int num) {
  vector<Station> stations_vec;
  for (int i = 0; i < num; ++i) {
    Station station;
    stationBucket.read(station, bucketID + i * sizeof(Station));
    stations_vec.push_back(station);
  }
  LOG("Queried " + std::to_string(num) +
      " stations from bucket ID: " + std::to_string(bucketID));
  return stations_vec;
}

int TicketBucketManager::addTickets(int num_days, int num_stations_per_day,
                                    int init_value) {
  int bucketID =
      ticketBucket.write(init_value, num_days * num_stations_per_day);
  LOG("Added tickets: " + std::to_string(num_days) + " days, " +
      std::to_string(num_stations_per_day) +
      " stations per day, bucket ID: " + std::to_string(bucketID));
  return bucketID;
}

vector<int> TicketBucketManager::queryTickets(int bucketID) {
  LOG("Querying all tickets from bucket ID: " + std::to_string(bucketID));
  return ticketBucket.read(bucketID);
}
vector<int> TicketBucketManager::queryTickets(int bucketID, int offset,
                                              int num_elements) {
  LOG("Querying " + std::to_string(num_elements) + " tickets from bucket ID: " +
      std::to_string(bucketID) + " offset: " + std::to_string(offset));
  return ticketBucket.read(bucketID, offset, num_elements);
}

void TicketBucketManager::updateTickets(int bucketID,
                                        const vector<int> &tickets) {
  LOG("Updating all tickets in bucket ID: " + std::to_string(bucketID));
  return ticketBucket.update(bucketID, tickets);
}

void TicketBucketManager::updateTickets(int bucketID, int offset,
                                        int num_elements,
                                        const vector<int> &tickets) {
  LOG("Updating " + std::to_string(num_elements) + " tickets in bucket ID: " +
      std::to_string(bucketID) + " offset: " + std::to_string(offset));
  return ticketBucket.update(bucketID, offset, num_elements, tickets);
}

TrainManager::TrainManager(const std::string &trainFile)
    : trainDB(trainFile + "_train", string32::string32_MAX()),
      ticketLookupDB(trainFile + "_ticket_lookup",
                     std::make_pair(ULONG_MAX, ULONG_MAX)),
      transferLookupDB(trainFile + "_transfer_lookup", ULONG_MAX),
      stationBucketManager(trainFile + "_station_bucket"),
      ticketBucketManager(trainFile + "_ticket_bucket") {
  LOG("TrainManager initialized with file prefix: " + trainFile);
}

int TrainManager::addTrain(const string32 &trainID, int stationNum_val,
                           int seatNum_val, const std::string &stations_str,
                           const std::string &prices_str,
                           const std::string &startTime_str,
                           const std::string &travelTimes_str,
                           const std::string &stopoverTimes_str,
                           const std::string &saleDates_str, char trainType) {
  LOG("Adding train: " + trainID.toString());

  if (!trainDB.find(trainID).empty()) {
    ERROR("Train ID already exists: " + trainID.toString());
    return -1; // Train ID already exists
  }

  vector<string32> stationNames = splitString(stations_str, '|');
  vector<int> prices = splitStringToInt(prices_str, '|');

  DateTime trainStartTime(sjtu::string32(startTime_str.c_str()),
                          true); // true for time
  if (!trainStartTime.hasTime()) {
    ERROR("Invalid start time format for train: " + trainID.toString());
    return -1;
  }

  vector<int> travelTimesMinutes = splitStringToInt(travelTimes_str, '|');

  vector<int> stopoverTimesMinutes;
  if (stopoverTimes_str == "_" && stationNum_val > 2) {
    stopoverTimesMinutes.assign(stationNum_val - 2, 0);
  } else {
    stopoverTimesMinutes = splitStringToInt(stopoverTimes_str, '|');
  }

  vector<string32> saleDateParts = splitString(saleDates_str, '|');
  if (saleDateParts.size() != 2) {
    ERROR("Invalid sale date format for train: " + trainID.toString());
    return -1; // Invalid sale date format
  }
  DateTime saleStartDt(saleDateParts[0]);
  DateTime saleEndDt(saleDateParts[1]);

  if (!saleStartDt.hasDate() || !saleEndDt.hasDate() ||
      saleStartDt.getDateMMDD() > saleEndDt.getDateMMDD()) {
    ERROR("Invalid sale dates for train: " + trainID.toString());
    return -1; // Invalid sale dates
  }

  if (stationNames.size() != static_cast<size_t>(stationNum_val) ||
      prices.size() != static_cast<size_t>(stationNum_val - 1) ||
      travelTimesMinutes.size() != static_cast<size_t>(stationNum_val - 1) ||
      (stationNum_val > 2 && stopoverTimesMinutes.size() !=
                                 static_cast<size_t>(stationNum_val - 2)) ||
      (stationNum_val == 2 && !stopoverTimesMinutes.empty() &&
       stopoverTimes_str != "_")) {
    ERROR("Invalid parameters size for train: " + trainID.toString());
    return -1;
  }
  if (stationNum_val == 2 && stopoverTimes_str != "_" &&
      !stopoverTimesMinutes.empty()) {
    ERROR("Invalid stopover times for 2-station train: " + trainID.toString());
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
    ERROR("Failed to add stations for train: " + trainID.toString());
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
  LOG("Successfully added train: " + trainID.toString() + " with " +
      std::to_string(stationNum_val) + " stations and " +
      std::to_string(seatNum_val) + " seats, starting at " +
      trainStartTime.toString() + " from " + saleStartDt.toString() + " to " +
      saleEndDt.toString());
  return 0;
}

int TrainManager::deleteTrain(const string32 &trainID) {
  LOG("Deleting train: " + trainID.toString());

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    ERROR("Train not found for deletion: " + trainID.toString());
    return -1;
  }
  Train trainToDelete = foundTrains[0];
  if (trainToDelete.isReleased) {
    ERROR("Cannot delete released train: " + trainID.toString());
    return -1; // Cannot delete a released train
  }

  trainDB.remove(trainID, trainToDelete);
  stationBucketManager.deleteStations(trainToDelete.stationBucketID,
                                      trainToDelete.stationNum);
  LOG("Successfully deleted train: " + trainID.toString());
  return 0;
}

int TrainManager::releaseTrain(const string32 &trainID) {
  LOG("Releasing train: " + trainID.toString());

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    ERROR("Train not found for release: " + trainID.toString());
    return -1; // Train not found
  }
  Train trainToRelease = foundTrains[0];
  if (trainToRelease.isReleased) {
    ERROR("Train already released: " + trainID.toString());
    return -1; // Already released
  }

  trainDB.remove(trainID, trainToRelease);
  trainToRelease.isReleased = true;
  int numSaleDays = calcDateDuration(trainToRelease.saleStartDate.getDateMMDD(),
                                     trainToRelease.saleEndDate.getDateMMDD()) +
                    1;

  int ticket_bID = ticketBucketManager.addTickets(
      numSaleDays, trainToRelease.stationNum - 1, trainToRelease.seatNum);

  if (ticket_bID == -1) {
    ERROR("Failed to add tickets for train: " + trainID.toString());
    return -1;
  }

  auto stations = stationBucketManager.queryStations(
      trainToRelease.stationBucketID, trainToRelease.stationNum);

  CustomStringHasher custom_hasher;
  size_t hashedStation[trainToRelease.stationNum];
  for (int i = 0; i < trainToRelease.stationNum; ++i) {
    hashedStation[i] = custom_hasher(stations[i].name.c_str());
  }

  for (int i = 0; i < trainToRelease.stationNum; ++i) {
    for (int j = i + 1; j < trainToRelease.stationNum; ++j) {
      ticketLookupDB.insert(std::make_pair(hashedStation[i], hashedStation[j]),
                            trainID); // from, to -> trainID
    }
    transferLookupDB.insert(hashedStation[i],
                            trainID); // fromStation -> trainID
  }

  trainToRelease.ticketBucketID = ticket_bID;
  trainDB.insert(trainID, trainToRelease);

  LOG("Successfully released train: " + trainID.toString() + " with " +
      std::to_string(numSaleDays) + " sale days");
  return 0;
}

std::string TrainManager::queryTrain(const string32 &trainID,
                                     const string32 &date_s32) {
  LOG("Querying train: " + trainID.toString() +
      " for date: " + date_s32.toString());

  DateTime queryDate(date_s32); // Parse "mm-dd" string
  if (!queryDate.hasDate()) {
    ERROR("Invalid date format for train query: " + date_s32.toString());
    return "-1"; // Invalid date format
  }

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    ERROR("Train not found for query: " + trainID.toString());
    return "-1"; // Train not found
  }
  Train train = foundTrains[0];

  if (queryDate.getDateMMDD() < train.saleStartDate.getDateMMDD() ||
      queryDate.getDateMMDD() > train.saleEndDate.getDateMMDD()) {
    ERROR("Train not on sale for date: " + trainID.toString() + " " +
          date_s32.toString());
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

  LOG("Successfully queried train: " + trainID.toString());
  return oss.str();
}

vector<TicketCandidate> TrainManager::querySingle(const string32 &from,
                                                  const string32 &to,
                                                  const DateTime &date,
                                                  const std::string &sortBy,
                                                  bool isTransfer) {
  LOG("Querying single route from " + from.toString() + " to " + to.toString() +
      " using sortBy: " + sortBy);
  CustomStringHasher custom_hasher;
  auto matchingTrainIDs = ticketLookupDB.find(
      std::make_pair(custom_hasher(from.c_str()), custom_hasher(to.c_str())));
  LOG("Found " + std::to_string(matchingTrainIDs.size()) +
      " matching trains for route from " + from.toString() + " to " +
      to.toString());
  if (matchingTrainIDs.empty()) {
    LOG("No matching trains found for route");
    return vector<TicketCandidate>(); // No matching trains found
  }
  vector<TicketCandidate>
      trainDetails; // trainID, price, from, to, duration, departureDateTime,
                    // endDateTime, seatNum
  for (const auto &trainID : matchingTrainIDs) {
    auto foundTrains = trainDB.find(trainID);
    if (foundTrains.empty() || !foundTrains[0].isReleased) {
      continue; // Skip unreleased trains
    }
    Train train = foundTrains[0];

    int from_idx = -1, to_idx = -1;
    bool flag = false;
    vector<Station> stations = stationBucketManager.queryStations(
        train.stationBucketID, train.stationNum);
    for (int i = 0; i < train.stationNum; ++i) {
      if (stations[i].name == from) {
        from_idx = i;
        flag = true;
      }
      if (stations[i].name == to) {
        to_idx = i;
        break;
      }
    }
    if (from_idx == -1 || to_idx == -1 || from_idx >= to_idx) {
      continue; // Invalid station names or indices
    }

    DateTime queryDate = date;
    queryDate.minusDuration((stations[from_idx].leavingTimeOffset +
                             train.startTime.getTimeMinutes()) /
                            1440 * 1440); // Adjust to train's start time
    if (isTransfer) {
      if (queryDate.getDateMMDD() < train.saleStartDate.getDateMMDD()) {
        queryDate = train.saleStartDate; // Use sale start date
      } else if (queryDate.getDateMMDD() > train.saleEndDate.getDateMMDD()) {
        continue;
      }
    } else {
      if (queryDate.getDateMMDD() < train.saleStartDate.getDateMMDD() ||
          queryDate.getDateMMDD() > train.saleEndDate.getDateMMDD()) {
        continue; // Not on sale on this date
      }
    }

    vector<int> leftSeats =
        queryLeftSeats(trainID, queryDate, from_idx, to_idx);
    int seatsAvailable = std::numeric_limits<int>::max();
    for (int seat : leftSeats) {
      seatsAvailable = std::min(seatsAvailable, seat);
    }

    int totalPrice = 0;
    for (int i = from_idx + 1; i <= to_idx; ++i) {
      totalPrice += stations[i].price;
    }

    int duration = stations[to_idx].arrivalTimeOffset -
                   stations[from_idx].leavingTimeOffset;

    DateTime departureDateTime(queryDate.getDateMMDD(),
                               train.startTime.getTimeMinutes());
    departureDateTime.addDuration(stations[from_idx].leavingTimeOffset);
    DateTime endDateTime(queryDate.getDateMMDD(),
                         train.startTime.getTimeMinutes());
    endDateTime.addDuration(stations[to_idx].arrivalTimeOffset);

    trainDetails.push_back(TicketCandidate(
        train.trainID, totalPrice, duration, stations[from_idx].name,
        stations[to_idx].name, departureDateTime, endDateTime, seatsAvailable));

    LOG("Found ticket candidate: " + train.trainID.toString() + " from " +
        stations[from_idx].name.toString() + " to " +
        stations[to_idx].name.toString() + " on " +
        departureDateTime.toString() + " with price " +
        std::to_string(totalPrice) + " and duration " +
        std::to_string(duration) + " minutes");
  }

  if (trainDetails.size() > 1) {
    if (sortBy == "cost") {
      std::sort(trainDetails.begin(), trainDetails.end(),
                [](const TicketCandidate &a, const TicketCandidate &b) {
                  if (a.price != b.price)
                    return a.price < b.price;
                  return a.trainID < b.trainID;
                });
    } else if (sortBy == "time") {
      std::sort(trainDetails.begin(), trainDetails.end(),
                [](const TicketCandidate &a, const TicketCandidate &b) {
                  if (a.duration != b.duration)
                    return a.duration < b.duration;
                  return a.trainID < b.trainID;
                });
    }
  }

  LOG("Found " + std::to_string(trainDetails.size()) + " ticket candidates");
  return trainDetails;
}

std::string TrainManager::queryTicket(const string32 &from, const string32 &to,
                                      const string32 &date_s32,
                                      const std::string &sortBy) {
  LOG("Querying tickets from " + from.toString() + " to " + to.toString() +
      " on " + date_s32.toString() + " sorted by " + sortBy);

  DateTime date(date_s32);
  auto trainDetails = querySingle(from, to, date, sortBy);
  if (trainDetails.empty()) {
    LOG("No tickets found for query");
    return "0"; // No tickets found
  }
  std::ostringstream oss;
  oss << trainDetails.size() << "\n";
  for (int i = 0; i < trainDetails.size(); ++i) {
    const auto &detail = trainDetails[i];
    oss << detail;
    if (i < trainDetails.size() - 1) {
      oss << "\n";
    }
  }

  LOG("Found " + std::to_string(trainDetails.size()) + " tickets");
  return oss.str();
}

std::string TrainManager::queryTransfer(const string32 &from,
                                        const string32 &to,
                                        const string32 &date_s32,
                                        const std::string &sortBy) {
  LOG("Querying transfer from " + from.toString() + " to " + to.toString() +
      " on " + date_s32.toString() + " sorted by " + sortBy);

  TicketCandidate bestLeg1Candidate;
  TicketCandidate bestLeg2Candidate;
  bool transferFound = false;

  // Variables for the best option
  int bestTotalPrice = std::numeric_limits<int>::max();
  string32 bestPrice_train1ID_tie = "";
  string32 bestPrice_train2ID_tie = "";

  int bestTotalDuration = std::numeric_limits<int>::max();
  string32 bestTime_train1ID_tie = "";
  string32 bestTime_train2ID_tie = "";

  CustomStringHasher custom_hasher;
  auto firstLegTrainIDs = transferLookupDB.find(custom_hasher(from.c_str()));

  for (const auto &train1ID : firstLegTrainIDs) {
    auto foundTrain1Vec = trainDB.find(train1ID);
    if (foundTrain1Vec.empty() || !foundTrain1Vec[0].isReleased) {
      continue;
    }
    Train train1_obj = foundTrain1Vec[0];
    vector<Station> stations_train1 = stationBucketManager.queryStations(
        train1_obj.stationBucketID, train1_obj.stationNum);

    int from_idx_train1 = -1;
    for (int i = 0; i < train1_obj.stationNum; ++i) {
      if (stations_train1[i].name == from) {
        from_idx_train1 = i;
        break;
      }
    }

    if (from_idx_train1 == -1) {
      continue;
    }

    for (int transfer_station_idx_train1 = from_idx_train1 + 1;
         transfer_station_idx_train1 < train1_obj.stationNum;
         ++transfer_station_idx_train1) {

      const Station &transferStation =
          stations_train1[transfer_station_idx_train1];

      int price_train1_leg = 0;
      for (int k = from_idx_train1 + 1; k <= transfer_station_idx_train1; ++k) {
        price_train1_leg += stations_train1[k].price;
      }

      DateTime queryDate(date_s32); // Parse "mm-dd" string
      queryDate.minusDuration(
          (stations_train1[from_idx_train1].leavingTimeOffset +
           train1_obj.startTime.getTimeMinutes()) /
          1440 * 1440); // Adjust to train's start time
      if (queryDate.getDateMMDD() < train1_obj.saleStartDate.getDateMMDD() ||
          queryDate.getDateMMDD() > train1_obj.saleEndDate.getDateMMDD()) {
        continue; // Not on sale on this date
      }

      // Now queryDate is the actual departure date of train1

      DateTime departureDateTime_train1_leg(
          queryDate.getDateMMDD(), train1_obj.startTime.getTimeMinutes());
      departureDateTime_train1_leg.addDuration(
          stations_train1[from_idx_train1].leavingTimeOffset);

      DateTime arrivalAtTransferDateTime_train1_leg(
          queryDate.getDateMMDD(), train1_obj.startTime.getTimeMinutes());
      arrivalAtTransferDateTime_train1_leg.addDuration(
          stations_train1[transfer_station_idx_train1].arrivalTimeOffset);

      int duration_train1_leg =
          stations_train1[transfer_station_idx_train1].arrivalTimeOffset -
          stations_train1[from_idx_train1].leavingTimeOffset;

      vector<int> leftSeats_train1_vec = queryLeftSeats(
          train1ID, queryDate, from_idx_train1, transfer_station_idx_train1);
      int seatsAvailable_train1_leg = std::numeric_limits<int>::max();

      for (int seat : leftSeats_train1_vec) {
        seatsAvailable_train1_leg = std::min(seatsAvailable_train1_leg, seat);
      }

      TicketCandidate ticket1(
          train1_obj.trainID, price_train1_leg, duration_train1_leg,
          stations_train1[from_idx_train1].name, transferStation.name,
          departureDateTime_train1_leg, arrivalAtTransferDateTime_train1_leg,
          seatsAvailable_train1_leg);

      vector<TicketCandidate> secondLegCandidates =
          querySingle(transferStation.name, to,
                      arrivalAtTransferDateTime_train1_leg, sortBy, true);

      DateTime nextDayDate = arrivalAtTransferDateTime_train1_leg;
      nextDayDate.addDuration(1440);
      vector<TicketCandidate> secondLegTommorowCandidates =
          querySingle(transferStation.name, to, nextDayDate, sortBy, true);

      for (const auto &ticket2 : secondLegCandidates) {
        if (ticket2.trainID == train1_obj.trainID) {
          continue;
        }

        if (!(ticket2.departureDateTime >=
              arrivalAtTransferDateTime_train1_leg)) {
          LOG("Skipping second leg: " + ticket2.trainID.toString() +
              " due to invalid departure time after first leg");
          continue;
        }

        int currentTotalPrice = ticket1.price + ticket2.price;
        int currentTotalDuration = ticket1.duration + ticket2.duration +
                                   ticket2.departureDateTime.calcDuration(
                                       arrivalAtTransferDateTime_train1_leg);

        if (sortBy == "cost") {
          // Cost as primary, time as secondary, train1 ID as tertiary, train2
          // ID as quaternary
          if (!transferFound || currentTotalPrice < bestTotalPrice ||
              (currentTotalPrice == bestTotalPrice &&
               currentTotalDuration < bestTotalDuration) ||
              (currentTotalPrice == bestTotalPrice &&
               currentTotalDuration == bestTotalDuration &&
               ticket1.trainID < bestPrice_train1ID_tie) ||
              (currentTotalPrice == bestTotalPrice &&
               currentTotalDuration == bestTotalDuration &&
               ticket1.trainID == bestPrice_train1ID_tie &&
               ticket2.trainID < bestPrice_train2ID_tie)) {
            bestTotalPrice = currentTotalPrice;
            bestTotalDuration = currentTotalDuration;
            bestPrice_train1ID_tie = ticket1.trainID;
            bestPrice_train2ID_tie = ticket2.trainID;
            bestLeg1Candidate = ticket1;
            bestLeg2Candidate = ticket2;
            transferFound = true;
          }
        } else if (sortBy == "time") {
          // Time as primary, cost as secondary, train1 ID as tertiary, train2
          // ID as quaternary
          if (!transferFound || currentTotalDuration < bestTotalDuration ||
              (currentTotalDuration == bestTotalDuration &&
               currentTotalPrice < bestTotalPrice) ||
              (currentTotalDuration == bestTotalDuration &&
               currentTotalPrice == bestTotalPrice &&
               ticket1.trainID < bestTime_train1ID_tie) ||
              (currentTotalDuration == bestTotalDuration &&
               currentTotalPrice == bestTotalPrice &&
               ticket1.trainID == bestTime_train1ID_tie &&
               ticket2.trainID < bestTime_train2ID_tie)) {
            bestTotalDuration = currentTotalDuration;
            bestTotalPrice = currentTotalPrice;
            bestTime_train1ID_tie = ticket1.trainID;
            bestTime_train2ID_tie = ticket2.trainID;
            bestLeg1Candidate = ticket1;
            bestLeg2Candidate = ticket2;
            transferFound = true;
          }
        }
      }

      for (const auto &ticket2 : secondLegTommorowCandidates) {
        if (ticket2.trainID == train1_obj.trainID) {
          continue;
        }

        if (!(ticket2.departureDateTime >=
              arrivalAtTransferDateTime_train1_leg)) {
          LOG("Skipping second leg: " + ticket2.trainID.toString() +
              " due to invalid departure time after first leg");
          continue;
        }

        int currentTotalPrice = ticket1.price + ticket2.price;
        int currentTotalDuration = ticket1.duration + ticket2.duration +
                                   ticket2.departureDateTime.calcDuration(
                                       arrivalAtTransferDateTime_train1_leg);

        if (sortBy == "cost") {
          // Cost as primary, time as secondary, train1 ID as tertiary, train2
          // ID as quaternary
          if (!transferFound || currentTotalPrice < bestTotalPrice ||
              (currentTotalPrice == bestTotalPrice &&
               currentTotalDuration < bestTotalDuration) ||
              (currentTotalPrice == bestTotalPrice &&
               currentTotalDuration == bestTotalDuration &&
               ticket1.trainID < bestPrice_train1ID_tie) ||
              (currentTotalPrice == bestTotalPrice &&
               currentTotalDuration == bestTotalDuration &&
               ticket1.trainID == bestPrice_train1ID_tie &&
               ticket2.trainID < bestPrice_train2ID_tie)) {
            bestTotalPrice = currentTotalPrice;
            bestTotalDuration = currentTotalDuration;
            bestPrice_train1ID_tie = ticket1.trainID;
            bestPrice_train2ID_tie = ticket2.trainID;
            bestLeg1Candidate = ticket1;
            bestLeg2Candidate = ticket2;
            transferFound = true;
          }
        } else if (sortBy == "time") {
          // Time as primary, cost as secondary, train1 ID as tertiary, train2
          // ID as quaternary
          if (!transferFound || currentTotalDuration < bestTotalDuration ||
              (currentTotalDuration == bestTotalDuration &&
               currentTotalPrice < bestTotalPrice) ||
              (currentTotalDuration == bestTotalDuration &&
               currentTotalPrice == bestTotalPrice &&
               ticket1.trainID < bestTime_train1ID_tie) ||
              (currentTotalDuration == bestTotalDuration &&
               currentTotalPrice == bestTotalPrice &&
               ticket1.trainID == bestTime_train1ID_tie &&
               ticket2.trainID < bestTime_train2ID_tie)) {
            bestTotalDuration = currentTotalDuration;
            bestTotalPrice = currentTotalPrice;
            bestTime_train1ID_tie = ticket1.trainID;
            bestTime_train2ID_tie = ticket2.trainID;
            bestLeg1Candidate = ticket1;
            bestLeg2Candidate = ticket2;
            transferFound = true;
          }
        }
      }
    }
  }

  if (!transferFound) {
    LOG("No transfer route found");
    return "";
  }

  std::ostringstream oss;
  oss << bestLeg1Candidate << "\n" << bestLeg2Candidate;
  LOG("Found transfer route with " + std::to_string(transferFound ? 2 : 0) +
      " legs");
  return oss.str();
}

/**
 * @brief Buy tickets for a specific train.
 * @return A tuple containing:
 * - Total price of the tickets
 * - Departure date of the train's START station in MMDD format
 * - Boolean indicating if the purchase was successful
 * - From station index (0-based)
 * - To station index (0-based)
 * - Offset of the departure time from the start of the day in minutes
 * - Offset of the arrival time at the destination station in minutes
 */
std::tuple<int, int, bool, int, int, int, int>
TrainManager::buyTicket(const string32 &trainID, const DateTime &departureDate,
                        int num, const string32 &from_station_name,
                        const string32 &to_station_name) {
  LOG("Buying " + std::to_string(num) + " tickets for train " +
      trainID.toString() + " from " + from_station_name.toString() + " to " +
      to_station_name.toString());

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty() || !foundTrains[0].isReleased) {
    ERROR("Train not found or not released: " + trainID.toString());
    return {-1, -1, false, -1, -1, -1, -1};
  }
  Train train = foundTrains[0];

  if (num > train.seatNum) {
    ERROR("Not enough seats available: requested " + std::to_string(num) +
          " but train has " + std::to_string(train.seatNum));
    return {-1, -1, false, -1, -1, -1, -1}; // Not enough seats available
  }

  int from_idx = -1;
  int to_idx = -1;

  vector<Station> stations = stationBucketManager.queryStations(
      train.stationBucketID, train.stationNum);
  for (int i = 0; i < train.stationNum; ++i) {
    if (train.stationBucketID == -1) {
      ERROR("No stations available for train: " + trainID.toString());
      return {-1, -1, false, -1, -1, -1, -1}; // No stations available
    }
    if (stations[i].name == from_station_name) {
      from_idx = i;
    }
    if (stations[i].name == to_station_name) {
      to_idx = i;
    }
  }
  if (from_idx == -1 || to_idx == -1 || from_idx >= to_idx) {
    ERROR("Invalid station names or indices for train: " + trainID.toString());
    return {-1, -1, false, -1, -1, -1, -1}; // Invalid station names or indices
  }

  DateTime queryDate = departureDate;
  queryDate.minusDuration((stations[from_idx].leavingTimeOffset +
                           train.startTime.getTimeMinutes()) /
                          1440 * 1440); // Adjust to train's start time
  if (queryDate.getDateMMDD() < train.saleStartDate.getDateMMDD() ||
      queryDate.getDateMMDD() > train.saleEndDate.getDateMMDD()) {
    ERROR("Train not on sale for date: " + trainID.toString());
    return {-1, -1, false, -1, -1, -1, -1}; // Not on sale on this date
  }

  bool flag = updateLeftSeats(trainID, queryDate, from_idx, to_idx, -num);

  int totalPrice = 0;
  for (int i = from_idx + 1; i <= to_idx; ++i) {
    totalPrice += stations[i].price;
  }

  totalPrice *= num; // Total price for the number of tickets

  if (flag) {
    LOG("Successfully bought " + std::to_string(num) + " tickets for " +
        std::to_string(totalPrice) + " total price");
  } else {
    ERROR("Failed to update seat availability for ticket purchase");
  }

  return {
      totalPrice,
      queryDate.getDateMMDD(),
      flag,
      from_idx,
      to_idx,
      train.startTime.getTimeMinutes() + stations[from_idx].leavingTimeOffset,
      train.startTime.getTimeMinutes() + stations[to_idx].arrivalTimeOffset};
}

bool TrainManager::refundTicket(const string32 &trainID,
                                const DateTime &departureDate, int num,
                                const int from_idx, const int to_idx) {
  LOG("Refunding " + std::to_string(num) + " tickets for train " +
      trainID.toString());

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty() || !foundTrains[0].isReleased) {
    ERROR("Train not found or not released for refund: " + trainID.toString());
    return false;
  }
  Train train = foundTrains[0];
  bool result = updateLeftSeats(trainID, departureDate, from_idx, to_idx, num);

  if (result) {
    LOG("Successfully refunded " + std::to_string(num) + " tickets");
  } else {
    ERROR("Failed to refund tickets");
  }

  return result;
}

vector<int> TrainManager::queryLeftSeats(const string32 &trainID, DateTime date,
                                         const int from_station_idx,
                                         const int to_station_idx) {
  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    ERROR("Train not found for seat query: " + trainID.toString());
    return vector<int>(); // No such train
  }
  Train train = foundTrains[0];
  if (!train.isReleased) {
    vector<int> seats(to_station_idx - from_station_idx, train.seatNum);
    LOG("Querying seats for unreleased train: " + trainID.toString());
    return seats;
  }

  int dayIndex =
      calcDateDuration(train.saleStartDate.getDateMMDD(), date.getDateMMDD());
  if (dayIndex < 0) {
    ERROR("Date is before sale starts for train: " + trainID.toString());
    return vector<int>(); // Date is before sale starts
  }

  int baseOffsetForDay = dayIndex * (train.stationNum - 1);
  int startOffsetInBucket = baseOffsetForDay + from_station_idx;

  int numElementsToQuery = to_station_idx - from_station_idx;
  if (numElementsToQuery <= 0) {
    ERROR("Invalid station indices for seat query");
    return vector<int>();
  }

  LOG("Querying left seats for train " + trainID.toString() + " from station " +
      std::to_string(from_station_idx) + " to " +
      std::to_string(to_station_idx));

  return ticketBucketManager.queryTickets(
      train.ticketBucketID, startOffsetInBucket, numElementsToQuery);
}

bool TrainManager::updateLeftSeats(const string32 &trainID, DateTime date,
                                   const int from_station_idx,
                                   const int to_station_idx, int num) {
  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    ERROR("Train not found for seat update: " + trainID.toString());
    return false; // No such train
  }
  Train train = foundTrains[0];
  if (!train.isReleased) {
    ERROR("Cannot update seats for unreleased train: " + trainID.toString());
    return false;
  }

  int dayIndex =
      calcDateDuration(train.saleStartDate.getDateMMDD(), date.getDateMMDD());
  if (dayIndex < 0) {
    ERROR("Date is before sale starts for seat update");
    return false; // Date is before sale starts
  }

  int baseOffsetForDay = dayIndex * (train.stationNum - 1);
  int startOffsetInBucket = baseOffsetForDay + from_station_idx;

  int numElementsToQuery = to_station_idx - from_station_idx;
  if (numElementsToQuery <= 0) {
    ERROR("Invalid station indices for seat update");
    return false;
  }

  auto tickets = ticketBucketManager.queryTickets(
      train.ticketBucketID, startOffsetInBucket, numElementsToQuery);

  for (int &ticket : tickets) {
    ticket += num; // Update the number of available seats
    LOG("Current available seats: " + std::to_string(ticket) +
        "; Previous: " + std::to_string(ticket - num));
    if (ticket < 0) {
      ERROR("Cannot have negative seats");
      return false; // Cannot have negative seats
    }
  }

  ticketBucketManager.updateTickets(train.ticketBucketID, startOffsetInBucket,
                                    numElementsToQuery, tickets);

  LOG("Updated seats for train " + trainID.toString() + " by " +
      std::to_string(num));
  return true;
}

#endif // TRAIN_MANAGER_HPP