#ifndef TRAIN_MANAGER_HPP
#define TRAIN_MANAGER_HPP

#include "storage/bptStorage.hpp"
#include "utils/string32.hpp"
#include "storage/cachedFileOperation.hpp"

using sjtu::string32;

struct Station {
    bool isStart;
    bool isEnd;
    string32 name;
    int price;
    int arrivalTime;
    int leavingTime;
};

struct Train {
    string32 trainID;
    int stationNum;
    int stationBucketID;
    int seatNum;

};

#endif // TRAIN_MANAGER_HPP