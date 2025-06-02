#include "../services/orderManager.hpp"
#include "../services/trainManager.hpp"
#include "../services/userManager.hpp"
#include "ApiHelpers.hpp"
#include "crow.h"

struct Context {
  UserManager user{"users"};
  TrainManager train{"trains"};
  OrderManager order{"orders", &train};
} ctx;

inline crow::json::wvalue ok() {
  crow::json::wvalue j;
  j["status"] = 0;
  return j;
}
inline crow::json::wvalue fail() {
  crow::json::wvalue j;
  j["status"] = -1;
  return j;
}

int main(int argc, char *argv[]) {
  crow::SimpleApp app;
  // add_user
  CROW_ROUTE(app, "/users")
      .methods("POST"_method)([](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body)
          return crow::response(400);
        bool okflag =
            ctx.user.addUser(static_cast<std::string>(body["cur_username"].s()),
                             static_cast<std::string>(body["username"].s()),
                             static_cast<std::string>(body["password"].s()),
                             static_cast<std::string>(body["name"].s()),
                             static_cast<std::string>(body["mailAddr"].s()),
                             body["privilege"].i());
        auto j = okflag ? ok() : fail();
        crow::response res{j};
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return res;
      });

  // login
  CROW_ROUTE(app, "/sessions")
      .methods("POST"_method)([](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body)
          return crow::response(400);
        bool okflag =
            ctx.user.login(static_cast<std::string>(body["username"].s()),
                           static_cast<std::string>(body["password"].s()));
        crow::response res{okflag ? ok() : fail()};
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return res;
      });

  // logout
  CROW_ROUTE(app, "/sessions/<string>")
      .methods("DELETE"_method)(
          [](const crow::request &, std::string username) {
            bool okflag = ctx.user.logout(username);
            return crow::response{okflag ? ok() : fail()};
          });

  // query_profile
  CROW_ROUTE(app, "/users/<string>")
      .methods("GET"_method)(
          [](const crow::request &req, std::string username) {
            auto cur = req.url_params.get("cur");
            if (!cur)
              return crow::response(400);
            auto ret = ctx.user.queryProfile(cur, username);
            if (ret == User())
              return crow::response{fail()};
            auto j = ok();
            j["profile"] = userToJson(ret);
            return crow::response{j};
          });

  // modify_profile
  CROW_ROUTE(app, "/users/<string>")
      .methods(
          "PUT"_method)([](const crow::request &req, std::string username) {
        auto body = crow::json::load(req.body);
        if (!body)
          return crow::response(400);
        auto r = ctx.user.modifyProfile(
            static_cast<std::string>(body["cur_username"].s()), username,
            body.has("password")
                ? static_cast<std::string>(body["password"].s())
                : "",
            body.has("name") ? static_cast<std::string>(body["name"].s()) : "",
            body.has("mailAddr")
                ? static_cast<std::string>(body["mailAddr"].s())
                : "",
            body.has("privilege") ? body["privilege"].i() : -1);
        if (r == User())
          return crow::response{fail()};
        auto j = ok();
        j["profile"] = userToJson(r);
        return crow::response{j};
      });

  // add_train
  CROW_ROUTE(app, "/trains")
      .methods("POST"_method)([](const crow::request &req) {
        auto b = crow::json::load(req.body);
        if (!b)
          return crow::response(400);
        int res = ctx.train.addTrain(
            static_cast<std::string>(b["trainID"].s()), b["stationNum"].i(),
            b["seatNum"].i(), static_cast<std::string>(b["stations"].s()),
            static_cast<std::string>(b["prices"].s()),
            static_cast<std::string>(b["startTime"].s()),
            static_cast<std::string>(b["travelTimes"].s()),
            static_cast<std::string>(b["stopoverTimes"].s()),
            static_cast<std::string>(b["saleDate"].s()),
            static_cast<std::string>(b["type"].s())[0]);
        return crow::response{res == 0 ? ok() : fail()};
      });

  // delete_train
  CROW_ROUTE(app, "/trains/<string>")
      .methods("DELETE"_method)([](const crow::request &, std::string id) {
        int r = ctx.train.deleteTrain(id);
        return crow::response{r == 0 ? ok() : fail()};
      });

  // release_train
  CROW_ROUTE(app, "/trains/<string>/release")
      .methods("POST"_method)([](const crow::request &, std::string id) {
        int r = ctx.train.releaseTrain(id);
        return crow::response{r == 0 ? ok() : fail()};
      });

  // query_train
  CROW_ROUTE(app, "/trains/<string>")
      .methods("GET"_method)([](const crow::request &req, std::string id) {
        auto date = req.url_params.get("date");
        if (!date)
          return crow::response(400);
        auto raw = ctx.train.queryTrain(id, date);
        if (raw.empty())
          return crow::response{fail()};
        auto j = ok();
        j["train"] = trainToJson(raw);
        return crow::response{j};
      });

  // query_ticket
  CROW_ROUTE(app, "/tickets")
      .methods("GET"_method)([](const crow::request &req) {
        auto f = req.url_params.get("from");
        auto t = req.url_params.get("to");
        auto d = req.url_params.get("date");
        if (!f || !t || !d)
          return crow::response(400);
        std::string sort =
            req.url_params.get("sort") ? req.url_params.get("sort") : "time";
        auto raw = ctx.train.queryTicket(f, t, d, sort);
        auto j = ok();
        j["ticket_list"] =
            raw.empty() ? crow::json::wvalue::list() : listToJson(raw);
        return crow::response{j};
      });

  // query_transfer
  CROW_ROUTE(app, "/transfer")
      .methods("GET"_method)([](const crow::request &req) {
        auto f = req.url_params.get("from");
        auto t = req.url_params.get("to");
        auto d = req.url_params.get("date");
        if (!f || !t || !d)
          return crow::response(400);
        std::string sort =
            req.url_params.get("sort") ? req.url_params.get("sort") : "time";
        auto raw = ctx.train.queryTransfer(f, t, d, sort);
        if (raw.empty())
          return crow::response{fail()};
        auto j = ok();
        j["transfer"] = listToJson(raw);
        return crow::response{j};
      });

  // buy_ticket
  CROW_ROUTE(app, "/orders/<string>/buy")
      .methods("POST"_method)([](const crow::request &req, std::string user) {
        if (!ctx.user.isLoggedIn(user))
          return crow::response{fail()};
        auto b = crow::json::load(req.body);
        if (!b)
          return crow::response(400);
        bool queue = b.has("queue") && b["queue"].b();
        int total = ctx.order.buyTicket(
            user, static_cast<std::string>(b["trainID"].s()),
            static_cast<std::string>(b["date"].s()), b["num"].i(),
            static_cast<std::string>(b["from"].s()),
            static_cast<std::string>(b["to"].s()), queue,
            /*timestamp*/ static_cast<int>(time(nullptr)));
        if (total == -1)
          return crow::response{fail()};
        auto j = ok();
        j["result"] = (total == 0 ? "queue" : std::to_string(total));
        return crow::response{j};
      });

  // query_order
  CROW_ROUTE(app, "/orders/<string>")
      .methods("GET"_method)([](const crow::request &, std::string user) {
        if (!ctx.user.isLoggedIn(user))
          return crow::response{fail()};
        auto vec = ctx.order.queryOrder(user);
        auto j = ok();
        j["orders"] = crow::json::wvalue::list();
        for (size_t i = 0; i < vec.size(); ++i)
          j["orders"][i] = orderToJson(vec[vec.size() - 1 - i]);
        return crow::response{j};
      });

  // refund_ticket
  CROW_ROUTE(app, "/orders/<string>")
      .methods("DELETE"_method)([](const crow::request &req, std::string user) {
        if (!ctx.user.isLoggedIn(user))
          return crow::response{fail()};
        int idx = 1;
        if (auto p = req.url_params.get("index"))
          idx = std::stoi(p);
        bool r = ctx.order.refundTicket(user, idx);
        return crow::response{r ? ok() : fail()};
      });

  CROW_ROUTE(app, "/system/clean").methods("POST"_method)([] {
    ctx.user.clean(); /*ctx.train.clean(); ctx.order.clean();*/
    return crow::response{ok()};
  });

  CROW_ROUTE(app, "/system/exit").methods("POST"_method)([] {
    ctx.user.clearLoggedInUsers();
    exit(0);
    return crow::response{}; // never reached
  });

  crow::logger::setLogLevel(crow::LogLevel::Debug);
  app.bindaddr("0.0.0.0").port(9988).run();
  return 0;
}