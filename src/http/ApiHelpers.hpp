#pragma once
#include "crow.h"
#include "services/orderManager.hpp"
#include "services/trainManager.hpp"
#include "services/userManager.hpp"
#include <sstream>
#include <string>
#include <vector>

// tiny helpers to convert the original text reply to JSON
inline crow::json::wvalue userToJson(const User &user) {
  crow::json::wvalue j;
  j["username"] = user.username.toString();
  j["name"] = user.name.toString();
  j["mailAddr"] = user.mailAddr.toString();
  j["privilege"] = user.privilege;
  return j;
}

inline crow::json::wvalue trainToJson(const std::string &raw) {
  std::istringstream iss(raw);
  std::string line;
  std::getline(iss, line);

  crow::json::wvalue root;
  std::istringstream header(line);
  std::string trainID, type;
  header >> trainID >> type;
  root["trainID"] = trainID;
  root["type"] = type;

  crow::json::wvalue stations = crow::json::wvalue::list();
  size_t idx = 0;
  while (std::getline(iss, line)) {
    if (line.empty())
      continue;
    std::istringstream ls(line);
    std::string station, arrival, arrow, leaving;
    int price, seat;
    ls >> station >> arrival >> arrow >> leaving >> price >> seat;
    crow::json::wvalue s;
    s["name"] = station;
    s["arrival_time"] = arrival;
    s["leaving_time"] = leaving;
    s["cumulative_price"] = price;
    s["seat_to_next"] = seat;
    stations[idx++] = std::move(s);
  }
  root["schedule"] = std::move(stations);
  return root;
}

inline crow::json::wvalue listToJson(const std::string &raw) {
  std::istringstream iss(raw);
  std::string line;
  crow::json::wvalue arr = crow::json::wvalue::list();
  size_t i = 0;
  while (std::getline(iss, line))
    if (!line.empty())
      arr[i++] = line;
  return arr;
}

inline crow::json::wvalue orderToJson(const Order &order) {
  crow::json::wvalue j;
  j["username"] = order.username.toString();
  j["trainID"] = order.trainID.toString();
  j["from_station_name"] = order.from_station_name.toString();
  j["from_station_idx"] = order.from_station_idx;
  j["to_station_name"] = order.to_station_name.toString();
  j["to_station_idx"] = order.to_station_idx;
  j["departureFromStation"] = order.departureFromStation.getDateString() + " " +
                              order.departureFromStation.getTimeString();
  j["arrivalAtStation"] = order.arrivalAtStation.getDateString() + " " +
                          order.arrivalAtStation.getTimeString();
  j["price"] = order.price;
  j["num"] = order.num;
  j["status"] = (order.status == SUCCESS
                     ? "success"
                     : (order.status == PENDING ? "pending" : "refunded"));
  j["timestamp"] = order.timestamp;
  return j;
}