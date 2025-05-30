#ifndef TRAIN_MANAGER_HPP
#define TRAIN_MANAGER_HPP

#include "stl/map.hpp"
#include "stl/vector.hpp"
#include "storage/bptStorage.hpp"
#include "storage/cachedFileOperation.hpp"
#include "storage/varLengthFileOperation.hpp"
#include "utils/dateFormatter.hpp"
#include "utils/dateTime.hpp"
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
  BPTStorage<string32, string32> ticketLookupDB; // stationName -> trainID
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

  vector<TicketCandidate> querySingle(const string32 &from, const string32 &to,
                                      const DateTime &date,
                                      const std::string &sortBy = "time");
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
      ticketLookupDB(trainFile + "_ticket_lookup", string32::string32_MAX()),
      stationBucketManager(trainFile + "_station_bucket_data"),
      ticketBucketManager(trainFile + "_ticket_bucket_data") {}

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

  trainDB.remove(trainID, trainToRelease);
  trainToRelease.isReleased = true;
  int numSaleDays = calcDateDuration(trainToRelease.saleStartDate.getDateMMDD(),
                                     trainToRelease.saleEndDate.getDateMMDD()) +
                    1;

  int ticket_bID = ticketBucketManager.addTickets(
      numSaleDays, trainToRelease.stationNum - 1, trainToRelease.seatNum);

  if (ticket_bID == -1)
    return -1;

  auto stations = stationBucketManager.queryStations(
      trainToRelease.stationBucketID, trainToRelease.stationNum);
  for (int i = 0; i < trainToRelease.stationNum; ++i) {
    // Update ticket lookup for each station
    ticketLookupDB.insert(stations[i].name, trainToRelease.trainID);
  }

  trainToRelease.ticketBucketID = ticket_bID;
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

vector<TicketCandidate> TrainManager::querySingle(const string32 &from,
                                                  const string32 &to,
                                                  const DateTime &date,
                                                  const std::string &sortBy) {
  // !important: FIX NEEDED:
  // 1. The query date should be the date of the train's START station. // FIXED

  DateTime queryDate = date;

  auto fromTrains = ticketLookupDB.find(from);
  auto toTrains = ticketLookupDB.find(to);

  map<string32, int> trainCountMap; // trainID -> count of matching trains
  vector<string32> matchingTrainIDs;
  for (const auto &trainID : fromTrains) {
    trainCountMap[trainID] = 1;
  }
  for (const auto &trainID : toTrains) {
    if (trainCountMap.find(trainID) != trainCountMap.end()) {
      matchingTrainIDs.push_back(trainID);
    }
  }

  if (matchingTrainIDs.empty()) {
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

    queryDate.minusDuration(
        (stations[from_idx].leavingTimeOffset + train.startTime.getTimeMinutes()) / 1440 * 1440); // Adjust to train's start time
    if (queryDate.getDateMMDD() < train.saleStartDate.getDateMMDD() ||
        queryDate.getDateMMDD() > train.saleEndDate.getDateMMDD()) {
      continue; // Not on sale on this date
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

    // trainDetails.push_back(
    //     std::make_tuple(train.trainID, totalPrice, stations[from_idx].name,
    //                     stations[to_idx].name, duration, departureDateTime,
    //                     endDateTime, seatsAvailable));
    trainDetails.push_back(TicketCandidate(
        train.trainID, totalPrice, duration, stations[from_idx].name,
        stations[to_idx].name, departureDateTime, endDateTime, seatsAvailable));
  }

  if (sortBy == "price") {
    std::sort(trainDetails.begin(), trainDetails.end(),
              [](const TicketCandidate &a, const TicketCandidate &b) {
                return a.price < b.price ||
                       (a.price == b.price && a.trainID < b.trainID);
              });
  } else if (sortBy == "time") {
    std::sort(trainDetails.begin(), trainDetails.end(),
              [](const TicketCandidate &a, const TicketCandidate &b) {
                return a.duration < b.duration ||
                       (a.duration == b.duration && a.trainID < b.trainID);
              });
  }
  return trainDetails;
}

std::string TrainManager::queryTicket(const string32 &from, const string32 &to,
                                      const string32 &date_s32,
                                      const std::string &sortBy) {
  DateTime date(date_s32);
  auto trainDetails = querySingle(from, to, date, sortBy);
  if (trainDetails.empty()) {
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

  return oss.str();
}

std::string TrainManager::queryTransfer(const string32 &from,
                                        const string32 &to,
                                        const string32 &date_s32,
                                        const std::string &sortBy) {
  DateTime queryDate(date_s32); // Parse "mm-dd" string

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

  auto firstLegTrainIDs = ticketLookupDB.find(from);

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
      for (int k = from_idx_train1; k < transfer_station_idx_train1; ++k) {
        price_train1_leg += stations_train1[k].price;
      }

      queryDate.minusDuration(
        (stations_train1[from_idx_train1].leavingTimeOffset + train1_obj.startTime.getTimeMinutes()) / 1440 * 1440); // Adjust to train's start time
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

      if (seatsAvailable_train1_leg <= 0) {
        continue; // No seats on this leg of train1
      }

      TicketCandidate ticket1(
          train1_obj.trainID, price_train1_leg, duration_train1_leg,
          stations_train1[from_idx_train1].name, transferStation.name,
          departureDateTime_train1_leg, arrivalAtTransferDateTime_train1_leg,
          seatsAvailable_train1_leg);

      vector<TicketCandidate> secondLegCandidates =
          querySingle(transferStation.name, to,
                      arrivalAtTransferDateTime_train1_leg, sortBy);

      for (const auto &ticket2 : secondLegCandidates) {
        if (ticket2.trainID == train1_obj.trainID) {
          continue;
        }

        if (!(ticket2.departureDateTime >
              arrivalAtTransferDateTime_train1_leg)) {
          continue;
        }

        if (sortBy == "price") {
          int currentTotalPrice = ticket1.price + ticket2.price;
          if (!transferFound || currentTotalPrice < bestTotalPrice) {
            bestTotalPrice = currentTotalPrice;
            bestPrice_train1ID_tie = ticket1.trainID;
            bestPrice_train2ID_tie = ticket2.trainID;
            bestLeg1Candidate = ticket1;
            bestLeg2Candidate = ticket2;
            transferFound = true;
          } else if (currentTotalPrice == bestTotalPrice) {
            if (ticket1.trainID < bestPrice_train1ID_tie) {
              bestPrice_train1ID_tie = ticket1.trainID;
              bestPrice_train2ID_tie = ticket2.trainID;
              bestLeg1Candidate = ticket1;
              bestLeg2Candidate = ticket2;
            } else if (ticket1.trainID == bestPrice_train1ID_tie &&
                       ticket2.trainID < bestPrice_train2ID_tie) {
              bestPrice_train2ID_tie = ticket2.trainID;
              bestLeg2Candidate = ticket2;
            }
          }
        } else if (sortBy == "time") {
          int currentTotalDuration = ticket1.duration + ticket2.duration;
          if (!transferFound || currentTotalDuration < bestTotalDuration) {
            bestTotalDuration = currentTotalDuration;
            bestTime_train1ID_tie = ticket1.trainID;
            bestTime_train2ID_tie = ticket2.trainID;
            bestLeg1Candidate = ticket1;
            bestLeg2Candidate = ticket2;
            transferFound = true;
          } else if (currentTotalDuration == bestTotalDuration) {
            if (ticket1.trainID < bestTime_train1ID_tie) {
              bestTime_train1ID_tie = ticket1.trainID;
              bestTime_train2ID_tie = ticket2.trainID;
              bestLeg1Candidate = ticket1;
              bestLeg2Candidate = ticket2;
            } else if (ticket1.trainID == bestTime_train1ID_tie &&
                       ticket2.trainID < bestTime_train2ID_tie) {
              bestTime_train2ID_tie = ticket2.trainID;
              bestLeg2Candidate = ticket2;
            }
          }
        }
      }
    }
  }

  if (!transferFound) {
    return "-1";
  }

  std::ostringstream oss;
  oss << bestLeg1Candidate << "\n" << bestLeg2Candidate;
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
  // !important: need fix here.
  // 1. Transform depatureDate to actual departure date of the TRAIN. // FIXED
  // 2. Return the departure date of TRAIN. // FIXED
  // 3. If NUM exceeds the number of seats available, return false. // FIXED

  DateTime queryDate = departureDate;

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty() || !foundTrains[0].isReleased)
    return {-1, -1, false, -1, -1, -1, -1};
  Train train = foundTrains[0];

  if (num > train.seatNum) {
    return {-1, -1, false, -1, -1, -1, -1}; // Not enough seats available
  }

  int from_idx = -1;
  int to_idx = -1;

  vector<Station> stations = stationBucketManager.queryStations(
      train.stationBucketID, train.stationNum);
  for (int i = 0; i < train.stationNum; ++i) {
    if (train.stationBucketID == -1) {
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
    return {-1, -1, false, -1, -1, -1, -1}; // Invalid station names or indices
  }

  queryDate.minusDuration(
        (stations[from_idx].leavingTimeOffset + train.startTime.getTimeMinutes()) / 1440 * 1440); // Adjust to train's start time
  if (queryDate.getDateMMDD() < train.saleStartDate.getDateMMDD() ||
      queryDate.getDateMMDD() > train.saleEndDate.getDateMMDD()) {
    return {-1, -1, false, -1, -1, -1, -1}; // Not on sale on this date
  }

  vector<int> leftSeats = queryLeftSeats(trainID, queryDate, from_idx, to_idx);
  bool flag = true;
  for (int seat : leftSeats) {
    if (seat < num) {
      flag = false;
    }
  }
  if (flag) {
    // we can buy the tickets
    vector<int> updatedSeats = leftSeats;
    for (int i = 0; i < leftSeats.size(); ++i) {
      updatedSeats[i] -= num;
    }

    int dayIndex = calcDateDuration(train.saleStartDate.getDateMMDD(),
                                    queryDate.getDateMMDD());
    int baseOffsetForDay =
        dayIndex * (train.stationNum - 1); // FIX: Maybe needed a fix here
    int startOffsetInBucket = baseOffsetForDay + from_idx;

    int numElementsToQuery = to_idx - from_idx;

    ticketBucketManager.updateTickets(train.ticketBucketID, startOffsetInBucket,
                                      numElementsToQuery, updatedSeats);
  }

  int totalPrice = 0;
  for (int i = from_idx + 1; i <= to_idx; ++i) {
    totalPrice += stations[i].price;
  }

  totalPrice *= num; // Total price for the number of tickets

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

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty() || !foundTrains[0].isReleased)
    return false;
  Train train = foundTrains[0];
  vector<int> leftSeats =
      queryLeftSeats(trainID, departureDate, from_idx, to_idx);
  if (leftSeats.empty() || from_idx < 0 || to_idx > train.stationNum ||
      from_idx >= to_idx) {
    return false;
  }
  for (int i = 0; i < to_idx - from_idx; ++i) {
    leftSeats[i] += num;
  }
  int dayIndex = calcDateDuration(train.saleStartDate.getDateMMDD(),
                                  departureDate.getDateMMDD());
  int baseOffsetForDay =
      dayIndex * (train.stationNum - 1); // FIX: Maybe needed a fix here
  int startOffsetInBucket = baseOffsetForDay + from_idx;

  int numElementsToQuery = to_idx - from_idx;

  ticketBucketManager.updateTickets(train.ticketBucketID, startOffsetInBucket,
                                    numElementsToQuery, leftSeats);

  return true;
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