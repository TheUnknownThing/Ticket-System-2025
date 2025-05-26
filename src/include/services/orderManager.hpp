#ifndef ORDER_MANAGER_HPP
#define ORDER_MANAGER_HPP

#include "stl/vector.hpp"
#include "storage/bptStorage.hpp"
#include "utils/string32.hpp"
#include <ostream>
#include <string>

using sjtu::string32;
using sjtu::vector;

enum OrderStatus { SUCCESS, PENDING, REFUNDED };

struct Order {
  string32 username;
  string32 trainID;
  string32 from;
  string32 to;
  int departureDateMMDD; // MMDD format of train depart from START station

  int price;
  int num;
  OrderStatus status;
  int timestamp;

  Order() = default;
  Order(const string32 &un, const string32 &tid, const string32 &fr,
        const string32 &t, int depDate, int pr, int n, OrderStatus st, int ts)
      : username(un), trainID(tid), from(fr), to(t), departureDateMMDD(depDate),
        price(pr), num(n), status(st), timestamp(ts) {}

  bool operator<(const Order &other) const {
    return timestamp < other.timestamp;
  }

  friend std::ostream &operator<<(std::ostream &os, const Order &order) {
    os << "[";
    switch (order.status) {
    case SUCCESS:
      os << "success";
      break;
    case PENDING:
      os << "pending";
      break;
    case REFUNDED:
      os << "refunded";
      break;
    }
    os << "] ";

    os << order.trainID << " " << order.from << " ";

    // LEAVING_TIME (MM-DD 00:00)
    os << (order.departureDateMMDD / 100 < 10 ? "0" : "")
       << order.departureDateMMDD / 100 << "-";
    os << (order.departureDateMMDD % 100 < 10 ? "0" : "")
       << order.departureDateMMDD % 100 << " ";
    os << "00:00"; // Placeholder, as leaving time is not stored

    // Output ->
    os << " -> ";

    // Output TO
    os << order.to << " ";

    // Output ARRIVING_TIME (MM-DD 00:00)
    os << (order.departureDateMMDD / 100 < 10 ? "0" : "")
       << order.departureDateMMDD / 100 << "-";
    os << (order.departureDateMMDD % 100 < 10 ? "0" : "")
       << order.departureDateMMDD % 100 << " ";
    os << "00:00"; // Placeholder, as arriving time is not stored

    // Output PRICE and NUM
    os << " " << order.price << " " << order.num;

    return os;
  }
};

class OrderManager {
private:
  BPTStorage<string32, Order> orderDB; // username -> orders
  BPTStorage<string32, string32>
      pendingQueue; // trainID -> orderID, it was naturally sorted by date
                    // (timestamp)

public:
  OrderManager(const std::string &orderFile, const std::string &queueFile);

  vector<Order> queryOrder(const string32 &username);
  int buyTicket(const string32 &username, const string32 &trainID,
                const string32 &date, int num, const string32 &from,
                const string32 &to, int price, bool queueIfNotAvailable,
                int timestamp);

  bool refundTicket(const string32 &username, int orderNum);

  void processPendingOrders(const string32 &trainID, const string32 &date);

private:
  /**
   * @brief Helper function to process a pending order when a ticket was
   * released.
   */
  void processPendingOrder(const string32 &trainID, const string32 &date);
};

OrderManager::OrderManager(const std::string &orderFile,
                           const std::string &queueFile)
    : orderDB(orderFile, string32::string32_MAX()),
      pendingQueue(queueFile, string32::string32_MAX()) {}

vector<Order> OrderManager::queryOrder(const string32 &username) {
  return orderDB.find(username);
}

#endif // ORDER_MANAGER_HPP