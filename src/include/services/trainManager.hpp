#ifndef TRAIN_MANAGER_HPP
#define TRAIN_MANAGER_HPP

#include "stl/vector.hpp"
#include "storage/bptStorage.hpp"
#include "storage/cachedFileOperation.hpp"
#include "utils/dateFormatter.hpp"
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
  int initialSeats;
};

class StationBucketManager {
private:
  CachedFileOperation<Station> stationBucket;

public:
  StationBucketManager() = delete;
  StationBucketManager(const std::string &stationFile);
  int addStations(
      vector<Station> &stations); // Pass by reference, num is stations.size()
  bool deleteStations(int bucketID, int num);
  vector<Station> queryStations(int bucketID, int num);
};

struct Train {
  string32 trainID;
  int stationNum;
  int stationBucketID;
  int seatNum;
  int saleDateStart; // MMDD format, 601 for 06-01
  int saleDateEnd;   // Also MMDD format
  int startTimeMinutes;
  char type;
  bool isReleased;

  bool operator==(const Train &other) const {
    return trainID == other.trainID;
  }
};

class TrainManager {
private:
  BPTStorage<string32, Train> trainDB;
  StationBucketManager stationBucketManager;

public:
  TrainManager(const std::string &trainFile, const std::string &stationFile);

  int addTrain(const string32 &trainID, int stationNum_val, int seatNum_val,
               const string32 &stations_str,      // -s ("s1|s2|s3")
               const string32 &prices_str,        // -p ("p1|p2")
               const string32 &startTime_str,     // -x ("hh:mm")
               const string32 &travelTimes_str,   // -t ("t1|t2")
               const string32 &stopoverTimes_str, // -o ("st1" or "_")
               const string32 &saleDates_str,     // -d ("d1|d2")
               char trainType);                   // -y

  int deleteTrain(const string32 &trainID);
  int releaseTrain(const string32 &trainID);
  std::string queryTrain(const string32 &trainID,
                         const string32 &date_str); // date_str is "mm-dd"
};

StationBucketManager::StationBucketManager(const std::string &stationFile)
    : stationBucket(stationFile + "_station") {}

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

// TrainManager implementations
TrainManager::TrainManager(const std::string &trainFile,
                           const std::string &stationFile)
    : trainDB(trainFile + "_train", string32::string32_MAX()), stationBucketManager(stationFile) {}

int TrainManager::addTrain(const string32 &trainID, int stationNum_val,
                           int seatNum_val, const string32 &stations_str,
                           const string32 &prices_str,
                           const string32 &startTime_str,
                           const string32 &travelTimes_str,
                           const string32 &stopoverTimes_str,
                           const string32 &saleDates_str, char trainType) {
  if (!trainDB.find(trainID).empty()) {
    return -1;
  }

  vector<string32> stationNames = splitS32String(stations_str, '|');
  vector<int> prices = splitS32StringToInt(prices_str, '|');
  int trainStartTimeMinutes = parseTimeToMinutes(startTime_str);
  vector<int> travelTimesMinutes = splitS32StringToInt(travelTimes_str, '|');
  vector<int> stopoverTimesMinutes =
      splitS32StringToInt(stopoverTimes_str, '|'); // Handles "_"

  vector<string32> saleDateParts = splitS32String(saleDates_str, '|');
  if (stationNames.size() != static_cast<size_t>(stationNum_val) ||
      prices.size() != static_cast<size_t>(stationNum_val - 1) ||
      travelTimesMinutes.size() != static_cast<size_t>(stationNum_val - 1) ||
      (stationNum_val > 2 && stopoverTimesMinutes.size() !=
                                 static_cast<size_t>(stationNum_val - 2)) ||
      (stationNum_val == 2 && !stopoverTimesMinutes.empty() &&
       stopoverTimes_str.toString() != "_") ||
      trainStartTimeMinutes == -1 || saleDateParts.size() != 2) {
    return -1;
  }
  if (stationNum_val == 2 && stopoverTimes_str.toString() != "_") {
    if (!stopoverTimesMinutes.empty())
      return -1;
  }

  int saleDateStart = parseDateToMMDD(saleDateParts[0]);
  int saleDateEnd = parseDateToMMDD(saleDateParts[1]);
  if (saleDateStart == -1 || saleDateEnd == -1 || saleDateStart > saleDateEnd) {
    return -1;
  }

  vector<Station> stationData;
  int currentRelativeTime = 0;

  for (int i = 0; i < stationNum_val; ++i) {
    Station s;
    s.name = stationNames[i];
    s.initialSeats = seatNum_val;

    if (i == 0) { // Start station
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
        s.leavingTimeOffset = -1;
      } else { // Intermediate station
        s.isEnd = false;
        currentRelativeTime += stopoverTimesMinutes[i - 1];
        s.leavingTimeOffset = currentRelativeTime;
      }
    }
    stationData.push_back(s);
  }

  int bucketID = stationBucketManager.addStations(stationData);
  if (bucketID == -1) {
    return -1;
  }

  Train newTrain;
  newTrain.trainID = trainID;
  newTrain.stationNum = stationNum_val;
  newTrain.stationBucketID = bucketID;
  newTrain.seatNum = seatNum_val;
  newTrain.saleDateStart = saleDateStart;
  newTrain.saleDateEnd = saleDateEnd;
  newTrain.startTimeMinutes = trainStartTimeMinutes;
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
    return -1;
  }
  Train trainToRelease = foundTrains[0];
  if (trainToRelease.isReleased) {
    return -1;
  }
  trainToRelease.isReleased = true;

  trainDB.remove(trainID, trainToRelease);
  trainDB.insert(trainID, trainToRelease);

  return 0;
}

std::string TrainManager::queryTrain(const string32 &trainID,
                                     const string32 &date_s32) {
  int queryDateMMDD = parseDateToMMDD(date_s32);
  if (queryDateMMDD == -1)
    return "-1"; // Invalid date format

  auto foundTrains = trainDB.find(trainID);
  if (foundTrains.empty()) {
    return "-1";
  }
  Train train = foundTrains[0];

  if (queryDateMMDD < train.saleDateStart ||
      queryDateMMDD > train.saleDateEnd) {
    return "-1"; // Not on sale on this date
  }
  std::ostringstream oss;
  oss << train.trainID.toString() << " " << train.type << "\n";

  vector<Station> stationDefs = stationBucketManager.queryStations(
      train.stationBucketID, train.stationNum);

  int cumulativePrice = 0;

  for (int i = 0; i < train.stationNum; ++i) {
    const Station &s = stationDefs[i];
    std::string arrivalTimeStr, leavingTimeStr;

    int actualArrivalDate = queryDateMMDD;
    int actualArrivalTimeOfDay = train.startTimeMinutes;

    int actualLeavingDate = queryDateMMDD;
    int actualLeavingTimeOfDay = train.startTimeMinutes;

    if (s.isStart) {
      arrivalTimeStr = "xx-xx xx:xx";

      actualLeavingDate = queryDateMMDD;
      actualLeavingTimeOfDay = train.startTimeMinutes;
      leavingTimeStr = formatDateFromMMDD(actualLeavingDate) + " " +
                       formatMinutesToTime(actualLeavingTimeOfDay);
      cumulativePrice = 0;
    } else {
      // Calculate absolute arrival time
      addDurationToDateTime(actualArrivalDate, actualArrivalTimeOfDay,
                            s.arrivalTimeOffset);
      arrivalTimeStr = formatDateFromMMDD(actualArrivalDate) + " " +
                       formatMinutesToTime(actualArrivalTimeOfDay);
      cumulativePrice += s.price;

      if (s.isEnd) {
        leavingTimeStr = "xx-xx xx:xx";
      } else {
        // Calculate absolute leaving time
        addDurationToDateTime(actualLeavingDate, actualLeavingTimeOfDay,
                              s.leavingTimeOffset);
        leavingTimeStr = formatDateFromMMDD(actualLeavingDate) + " " +
                         formatMinutesToTime(actualLeavingTimeOfDay);
      }
    }

    oss << s.name.toString() << " " << arrivalTimeStr << " -> "
        << leavingTimeStr << " " << cumulativePrice << " ";

    if (s.isEnd) {
      oss << "x";
    } else {
      // NOTE: NEED IMPLEMENT
      oss << train.seatNum;
    }

    if (!s.isEnd) {
      oss << "\n";
    }
  }
  return oss.str();
}

#endif // TRAIN_MANAGER_HPP
