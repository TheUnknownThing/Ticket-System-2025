#ifndef TRAIN_MANAGER_H
#define TRAIN_MANAGER_H

#include "storage/bptStorage.hpp"
#include "utils/string64.hpp"
#include "stl/vector.hpp"

using sjtu::vector;
using sjtu::string64;

struct Station {
    std::string name;
    int arrivalTime; // minutes since start
    int departureTime; // minutes since start
    int price; // cumulative price from start
};

struct Train {
    std::string trainID;
    int stationNum;
    std::vector<Station> stations;
    int seatNum;
    std::string startTime;
    std::vector<int> travelTimes;
    std::vector<int> stopoverTimes;
    std::string saleDateBegin;
    std::string saleDateEnd;
    char type;
    bool released;
    
    // For each day and station, store remaining seats
    // Implemented as a separate B+ tree
};

class TrainManager {
private:
    BPTStorage<std::string, Train> trainDB;
    BPTStorage<std::string, int> stationIndex; // stationName -> trainID
    BPTStorage<std::pair<std::string, std::string>, int> seatDB; // (trainID+date) -> seat data
    
public:
    TrainManager(const std::string& trainFile, const std::string& stationFile, 
                const std::string& seatFile);
    
    bool addTrain(const std::string& trainID, int stationNum, int seatNum,
                 const std::vector<std::string>& stations,
                 const std::vector<int>& prices,
                 const std::string& startTime,
                 const std::vector<int>& travelTimes,
                 const std::vector<int>& stopoverTimes,
                 const std::string& saleDateBegin,
                 const std::string& saleDateEnd,
                 char type);
    
    bool deleteTrain(const std::string& trainID);
    bool releaseTrain(const std::string& trainID);
    
    Train queryTrain(const std::string& trainID);
    std::vector<std::pair<Train, int>> queryTicket(const std::string& from, 
                                                 const std::string& to,
                                                 const std::string& date,
                                                 const std::string& sortBy);
    
    std::pair<Train, Train> queryTransfer(const std::string& from, 
                                        const std::string& to,
                                        const std::string& date,
                                        const std::string& sortBy);
    
    int buyTicket(const std::string& username, const std::string& trainID,
                 const std::string& date, int num, 
                 const std::string& from, const std::string& to,
                 bool queueIfNotAvailable);
    
    bool refundTicket(const std::string& username, int orderNum);
    
private:
    void updateSeatData(const std::string& trainID, const std::string& date,
                       int fromIdx, int toIdx, int num);
    
    std::vector<int> getSeatData(const std::string& trainID, const std::string& date);
};

#endif // TRAIN_MANAGER_H