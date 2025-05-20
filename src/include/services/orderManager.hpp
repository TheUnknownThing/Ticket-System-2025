#ifndef ORDER_MANAGER_HPP
#define ORDER_MANAGER_HPP

#include "stl/vector.hpp"
#include "storage/bptStorage.hpp"
#include "utils/string32.hpp"
#include <string>

using sjtu::string32;

enum OrderStatus { SUCCESS, PENDING, REFUNDED };

struct Order {
  string32 username;
  string32 trainID;
  string32 from;
  string32 to;
  int departureDateMMDD;   // MMDD format
  int leavingTimeMinutes;  // Minutes from midnight
  int arrivalDateMMDD;     // MMDD format
  int arrivingTimeMinutes; // Minutes from midnight
  int price;
  int num;
  OrderStatus status;
  int timestamp;

  Order() = default;
  Order(const string32 &un, const string32 &tid, const string32 &fr,
        const string32 &t, int depDate, int depTime, int arrDate, int arrTime,
        int pr, int n, OrderStatus st, int ts)
      : username(un), trainID(tid), from(fr), to(t), departureDateMMDD(depDate),
        leavingTimeMinutes(depTime), arrivalDateMMDD(arrDate),
        arrivingTimeMinutes(arrTime), price(pr), num(n), status(st),
        timestamp(ts) {}
};

class OrderManager {
private:
  BPTStorage<string32, Order> orderDB; // username -> orders
  BPTStorage<std::pair<string32, int>, string32>
      pendingQueue; // (trainID+timeStamp) -> orderID

public:
  OrderManager(const std::string &orderFile, const std::string &queueFile);

  std::vector<Order> queryOrder(const string32 &username);
  int buyTicket(const string32 &username, const string32 &trainID,
                const string32 &date, int num, const string32 &from,
                const string32 &to, int price, bool queueIfNotAvailable,
                int timestamp);

  bool refundTicket(const string32 &username, int orderNum);

  void processPendingOrders(const string32 &trainID, const string32 &date);
};

OrderManager::OrderManager(const std::string &orderFile,
                               const std::string &queueFile)
    : orderDB(orderFile, string32::string32_MAX()),
      pendingQueue(queueFile, std::make_pair(string32::string32_MAX(), INT_MAX)) {}



#endif // ORDER_MANAGER_HPP