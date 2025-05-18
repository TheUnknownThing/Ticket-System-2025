#ifndef ORDER_MANAGER_HPP
#define ORDER_MANAGER_HPP

#include "storage/bptStorage.hpp"
#include "utils/string64.hpp"
#include <string>
#include "stl/vector.hpp"

using sjtu::string64;

enum OrderStatus {
    SUCCESS,
    PENDING,
    REFUNDED
};

struct Order {
    string64 username;
    string64 trainID;
    string64 from;
    string64 to;
    string64 date;
    string64 leavingTime;
    string64 arrivingTime;
    int price;
    int num;
    OrderStatus status;
    int timestamp;
    
    Order() = default;
    Order(const string64& un, const string64& tid, const string64& fr,
         const string64& t, const string64& dt, const string64& lt,
         const string64& at, int pr, int n, OrderStatus st, int ts)
        : username(un), trainID(tid), from(fr), to(t), date(dt), leavingTime(lt),
          arrivingTime(at), price(pr), num(n), status(st), timestamp(ts) {}
};

class OrderManager {
private:
    BPTStorage<string64, Order> orderDB; // username -> orders
    BPTStorage<std::pair<string64, string64>, string64> pendingQueue; // (trainID+date) -> username+orderID
    
public:
    OrderManager(const string64& orderFile, const string64& queueFile);
    
    std::vector<Order> queryOrder(const string64& username);
    int buyTicket(const string64& username, const string64& trainID,
                 const string64& date, int num, 
                 const string64& from, const string64& to,
                 int price, bool queueIfNotAvailable, int timestamp);
    
    bool refundTicket(const string64& username, int orderNum);
    
    void processPendingOrders(const string64& trainID, const string64& date);
};

#endif // ORDER_MANAGER_HPP