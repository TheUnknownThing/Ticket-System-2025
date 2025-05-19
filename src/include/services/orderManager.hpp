#ifndef ORDER_MANAGER_HPP
#define ORDER_MANAGER_HPP

#include "storage/bptStorage.hpp"
#include "utils/string32.hpp"
#include <string>
#include "stl/vector.hpp"

using sjtu::string32;

enum OrderStatus {
    SUCCESS,
    PENDING,
    REFUNDED
};

struct Order {
    string32 username;
    string32 trainID;
    string32 from;
    string32 to;
    string32 date;
    string32 leavingTime;
    string32 arrivingTime;
    int price;
    int num;
    OrderStatus status;
    int timestamp;
    
    Order() = default;
    Order(const string32& un, const string32& tid, const string32& fr,
         const string32& t, const string32& dt, const string32& lt,
         const string32& at, int pr, int n, OrderStatus st, int ts)
        : username(un), trainID(tid), from(fr), to(t), date(dt), leavingTime(lt),
          arrivingTime(at), price(pr), num(n), status(st), timestamp(ts) {}
};

class OrderManager {
private:
    BPTStorage<string32, Order> orderDB; // username -> orders
    BPTStorage<std::pair<string32, string32>, string32> pendingQueue; // (trainID+date) -> username+orderID
    
public:
    OrderManager(const string32& orderFile, const string32& queueFile);
    
    std::vector<Order> queryOrder(const string32& username);
    int buyTicket(const string32& username, const string32& trainID,
                 const string32& date, int num, 
                 const string32& from, const string32& to,
                 int price, bool queueIfNotAvailable, int timestamp);
    
    bool refundTicket(const string32& username, int orderNum);
    
    void processPendingOrders(const string32& trainID, const string32& date);
};

#endif // ORDER_MANAGER_HPP