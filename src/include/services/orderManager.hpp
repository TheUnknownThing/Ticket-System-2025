#ifndef ORDER_MANAGER_HPP
#define ORDER_MANAGER_HPP

#include "services/trainManager.hpp"
#include "services/userManager.hpp"
#include "stl/vector.hpp"
#include "storage/bptStorage.hpp"
#include "utils/dateTime.hpp"
#include "utils/logger.hpp"
#include "utils/string32.hpp"
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>

using sjtu::string32;
using sjtu::vector;

enum OrderStatus { SUCCESS, PENDING, REFUNDED };

struct Order {
  string32 username;
  string32 trainID;
  string32 from_station_name;
  int from_station_idx; // 0-based index
  string32 to_station_name;
  int to_station_idx; // 0-based index
  DateTime
      departureDateTime; // DateTime train departs from train's START station.
  DateTime departureFromStation; // DateTime train departs from this leg's
                                 // departure station.
  DateTime arrivalAtStation;

  int price;
  int num;
  OrderStatus status;
  int timestamp;

  Order() = default;
  Order(const string32 &un, const string32 &tid, const string32 &fr_name,
        int fr_idx, const string32 &to_name, int to_idx,
        const DateTime &depDateTime, const DateTime &depFromStation,
        const DateTime &arrAtStation, int pr, int n, OrderStatus st, int ts)
      : username(un), trainID(tid), from_station_name(fr_name),
        from_station_idx(fr_idx), to_station_name(to_name),
        to_station_idx(to_idx), departureDateTime(depDateTime),
        departureFromStation(depFromStation), arrivalAtStation(arrAtStation),
        price(pr), num(n), status(st), timestamp(ts) {}

  bool operator<(const Order &other) const {
    return timestamp < other.timestamp;
  }

  bool operator>(const Order &other) const {
    return timestamp > other.timestamp;
  }

  bool operator<=(const Order &other) const {
    return timestamp <= other.timestamp;
  }

  bool operator==(const Order &other) const {
    return username == other.username && trainID == other.trainID &&
           timestamp == other.timestamp; // faster, do not compare all fields
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

    os << order.trainID.toString() << " " << order.from_station_name.toString()
       << " " << order.departureFromStation.getDateString() << " "
       << order.departureFromStation.getTimeString() << " -> ";

    os << order.to_station_name.toString() << " "
       << order.arrivalAtStation.getDateString() << " "
       << order.arrivalAtStation.getTimeString();

    os << " " << order.price / order.num << " " << order.num;

    return os;
  }
};

class OrderManager {
private:
  BPTStorage<string32, Order> orderDB; // username -> Order
  BPTStorage<std::pair<string32, int>, Order>
      pendingQueue; // <trainID, depDate> -> Order

  TrainManager *trainManager_ptr;

public:
  OrderManager() = delete;
  OrderManager(const std::string &orderFile, TrainManager *tm);

  vector<Order> queryOrder(const string32 &username);

  /**
   * @brief Attempts to buy a ticket.
   * @param username User requesting the ticket.
   * @param trainID Train ID.
   * @param date_str Date of departure from the TRAIN'S STARTING STATION (format
   * "MM-DD").
   * @param num_tickets Number of tickets.
   * @param from_station_name Name of the departure station for this leg.
   * @param to_station_name Name of the arrival station for this leg.
   * @param queueIfNotAvailable If true, add to pending queue if tickets not
   * immediately available.
   * @param timestamp A unique timestamp for this buy attempt.
   * @return price for success, 0 for pending, -1 for failure (e.g. invalid
   * input, or not queued on failure). Note: The `price` is assumed to be
   * determined during the (simulated) call to TrainManager.
   */
  int buyTicket(const string32 &username, const string32 &trainID,
                const string32 &date_str, int num_tickets,
                const string32 &from_station_name,
                const string32 &to_station_name, bool queueIfNotAvailable,
                int timestamp);

  /**
   * @brief Refunds a ticket.
   * @param username User requesting the refund.
   * @param orderIndex 1-based index of the order in the user's `queryOrder`
   * list (sorted by timestamp descending).
   * @return True if refund successful or order already refunded, false
   * otherwise.
   */
  bool refundTicket(const string32 &username, int orderIndex);

private:
  /**
   * @brief Processes pending orders for a given train on a specific departure
   * date (from train's origin).
   * @param trainID The train ID.
   * @param date_str The train's departure date from its STARTING STATION
   * (format "MM-DD").
   */
  void processPendingOrders(const string32 &trainID, int origin_date_mmdd);
};

OrderManager::OrderManager(const std::string &orderFile, TrainManager *tm)
    : orderDB(orderFile + "_order", string32::string32_MAX()),
      pendingQueue(orderFile + "_pending",
                   std::make_pair(string32::string32_MAX(), INT_MAX)),
      trainManager_ptr(tm) {
  LOG("OrderManager initialized with file: " + orderFile);
}

vector<Order> OrderManager::queryOrder(const string32 &username) {
  LOG("Querying orders for user: " + username.toString());
  vector<Order> orders = orderDB.find(username);
  LOG("Found " + std::to_string(orders.size()) +
      " orders for user: " + username.toString());
  return orders;
}

int OrderManager::buyTicket(const string32 &username, const string32 &trainID,
                            const string32 &date_str, int num_tickets,
                            const string32 &from_station_name,
                            const string32 &to_station_name,
                            bool queueIfNotAvailable, int timestamp) {

  LOG("Buy ticket request - User: " + username.toString() +
      ", Train: " + trainID.toString() + ", Date: " + date_str.toString() +
      ", Tickets: " + std::to_string(num_tickets) + ", From: " +
      from_station_name.toString() + ", To: " + to_station_name.toString());

  DateTime trainOriginDepDate(date_str); // Parses "MM-DD"
  if (!trainOriginDepDate.hasDate()) {
    ERROR("Invalid date format: " + date_str.toString());
    return -1; // Invalid date
  }
  if (num_tickets <= 0) {
    ERROR("Invalid number of tickets: " + std::to_string(num_tickets));
    return -1; // Invalid number of tickets
  }

  auto [price, origin_date_mmdd, isSuccessful, from_idx, to_idx, depTimeOffset,
        arrTimeOffset] =
      trainManager_ptr->buyTicket(trainID, trainOriginDepDate, num_tickets,
                                  from_station_name, to_station_name);

  if (price != -1 && origin_date_mmdd != -1 && isSuccessful) {
    DateTime departureFromStation = DateTime(origin_date_mmdd);
    departureFromStation.addDuration(depTimeOffset);
    DateTime arrivalAtStation = DateTime(origin_date_mmdd);
    arrivalAtStation.addDuration(arrTimeOffset);
    Order newOrder(username, trainID, from_station_name, from_idx,
                   to_station_name, to_idx, DateTime(origin_date_mmdd),
                   departureFromStation, arrivalAtStation, price, num_tickets,
                   SUCCESS, timestamp);
    orderDB.insert(username, newOrder);
    LOG("Ticket purchase successful - User: " + username.toString() +
        ", Train: " + trainID.toString() + ", Price: " + std::to_string(price));
    return price;
  } else {
    if (price != -1 && origin_date_mmdd != -1 && !isSuccessful &&
        queueIfNotAvailable) {
      DateTime departureFromStation = DateTime(origin_date_mmdd);
      departureFromStation.addDuration(depTimeOffset);
      DateTime arrivalAtStation = DateTime(origin_date_mmdd);
      arrivalAtStation.addDuration(arrTimeOffset);
      Order newOrder(username, trainID, from_station_name, from_idx,
                     to_station_name, to_idx, DateTime(origin_date_mmdd),
                     departureFromStation, arrivalAtStation, price, num_tickets,
                     PENDING, timestamp);
      orderDB.insert(username, newOrder);
      pendingQueue.insert(std::make_pair(trainID, origin_date_mmdd), newOrder);
      LOG("Ticket purchase queued - User: " + username.toString() +
          ", Train: " + trainID.toString());
      return 0; // Pending
    } else {
      ERROR("Ticket purchase failed - User: " + username.toString() +
            ", Train: " + trainID.toString());
      return -1; // Failure
    }
  }
}

bool OrderManager::refundTicket(const string32 &username,
                                int orderIndex) { // orderIndex is 1-based
  LOG("Refund ticket request - User: " + username.toString() +
      ", Order index: " + std::to_string(orderIndex));

  if (orderIndex <= 0) {
    ERROR("Invalid order index: " + std::to_string(orderIndex));
    return false;
  }

  vector<Order> userOrders = orderDB.find(username);

  if (static_cast<size_t>(orderIndex) > userOrders.size()) {
    ERROR("Order index out of range - User: " + username.toString() +
          ", Index: " + std::to_string(orderIndex) +
          ", Total orders: " + std::to_string(userOrders.size()));
    return false;
  }

  Order orderToRefund = userOrders[userOrders.size() - orderIndex];

  // std::cout << "Refunding order: " << orderToRefund << std::endl;

  if (orderToRefund.status == REFUNDED) {
    LOG("Order already refunded - User: " + username.toString() +
        ", Train: " + orderToRefund.trainID.toString());
    return false; // Already refunded
  }

  auto originalState = orderToRefund.status;

  if (orderToRefund.status == SUCCESS || orderToRefund.status == PENDING) {
    orderDB.remove(username, orderToRefund);
    if (originalState == PENDING) {
      pendingQueue.remove(
          std::make_pair(orderToRefund.trainID,
                         orderToRefund.departureDateTime.getDateMMDD()),
          orderToRefund);
    }
    orderToRefund.status = REFUNDED;
    orderDB.insert(username, orderToRefund);

    if (originalState == SUCCESS) {
      int from_idx = orderToRefund.from_station_idx;
      int to_idx = orderToRefund.to_station_idx;
      if (from_idx != -1 && to_idx != -1) {
        trainManager_ptr->refundTicket(orderToRefund.trainID,
                                       orderToRefund.departureDateTime,
                                       orderToRefund.num, from_idx, to_idx);
      }

      processPendingOrders(orderToRefund.trainID,
                           orderToRefund.departureDateTime.getDateMMDD());
    }

    LOG("Ticket refund successful - User: " + username.toString() +
        ", Train: " + orderToRefund.trainID.toString() +
        ", Status was: " + (originalState == SUCCESS ? "SUCCESS" : "PENDING"));
    return true;
  }

  ERROR("Refund failed - invalid order status");
  return false;
}

void OrderManager::processPendingOrders(const string32 &trainID,
                                        int origin_date_mmdd) {
  LOG("Processing pending orders for train: " + trainID.toString() +
      ", Date: " + std::to_string(origin_date_mmdd));

  vector<Order> candidates =
      pendingQueue.find(std::make_pair(trainID, origin_date_mmdd));

  LOG("Found " + std::to_string(candidates.size()) +
      " pending orders to process");

  int processedCount = 0;
  for (const Order &pendingOrder : candidates) {
    bool success = trainManager_ptr->updateLeftSeats(
        pendingOrder.trainID, pendingOrder.departureDateTime,
        pendingOrder.from_station_idx, pendingOrder.to_station_idx,
        -pendingOrder.num);

    if (success) {
      pendingQueue.remove(std::make_pair(trainID, origin_date_mmdd),
                          pendingOrder);
      orderDB.remove(pendingOrder.username, pendingOrder);
      Order updatedOrder = pendingOrder;
      updatedOrder.status = SUCCESS;
      orderDB.insert(updatedOrder.username, updatedOrder);
      processedCount++;

      LOG("Pending order promoted to SUCCESS - User: " +
          pendingOrder.username.toString() +
          ", Train: " + pendingOrder.trainID.toString());
    }
  }

  LOG("Processed " + std::to_string(processedCount) +
      " pending orders successfully");
}

#endif // ORDER_MANAGER_HPP