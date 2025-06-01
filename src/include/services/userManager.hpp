#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include "stl/map.hpp"
#include "storage/bptStorage.hpp"
#include "utils/string32.hpp"
#include <string>

using sjtu::map;
using sjtu::string32;

struct User {
  string32 username;
  string32 password;
  string32 name;
  string32 mailAddr;
  int privilege;

  User() = default;
  User(const string32 &un, const string32 &pw, const string32 &nm,
       const string32 &ma, int priv)
      : username(un), password(pw), name(nm), mailAddr(ma), privilege(priv) {}

  bool operator==(const User &other) const {
    return username == other.username;
  }

  bool operator!=(const User &other) const { return !(*this == other); }

  bool operator<=(const User &other) const {
    return username <= other.username;
  }

  bool operator>(const User &other) const {
    return username > other.username;
  }
  bool operator<(const User &other) const {
    return username < other.username;
  }

  friend std::ostream &operator<<(std::ostream &os, const User &user) {
    if (user == User())
      return os << "-1";
    os << user.username << " " << user.name << " " << user.mailAddr << " "
       << user.privilege;
    return os;
  }
};

class UserManager {
private:
  BPTStorage<string32, User> userDB;
  map<string32, int> loggedInUsers; // username -> privilege

public:
  UserManager(const std::string &filename);

  bool addUser(const string32 &curUser, const string32 &username,
               const string32 &password, const string32 &name,
               const string32 &mailAddr, int privilege);

  bool login(const string32 &username, const string32 &password);
  bool logout(const string32 &username);

  User queryProfile(const string32 &curUser, const string32 &username);
  User modifyProfile(const string32 &curUser, const string32 &username,
                     const string32 &password, const string32 &name,
                     const string32 &mailAddr, int privilege);

  void clean();
  void clearLoggedInUsers();
  
  bool isLoggedIn(const string32 &username) const;

private:
  int getPrivilege(const string32 &username) const;

  bool isFirstUser() const;
};

UserManager::UserManager(const std::string &filename)
    : userDB(filename + "_user", string32::string32_MAX()) {}

bool UserManager::addUser(const string32 &curUser, const string32 &username,
                          const string32 &password, const string32 &name,
                          const string32 &mailAddr, int privilege) {
  if (userDB.find(username).size() > 0)
    return false;

  if (!isFirstUser()) {
    if (!isLoggedIn(curUser))
      return false;
    if (getPrivilege(curUser) <= privilege)
      return false;
  } else {
    privilege = 10; // First user gets max privilege
  }

  User newUser(username, password, name, mailAddr, privilege);
  userDB.insert(username, newUser);
  return true;
}

bool UserManager::login(const string32 &username, const string32 &password) {
  auto users = userDB.find(username);
  if (users.empty() || users[0].password != password)
    return false;
  if (isLoggedIn(username))
    return false;

  loggedInUsers[username] = users[0].privilege;
  return true;
}

bool UserManager::logout(const string32 &username) {
  if (!isLoggedIn(username))
    return false;
  loggedInUsers.erase(loggedInUsers.find(username));
  return true;
}

User UserManager::queryProfile(const string32 &curUser,
                               const string32 &username) {
  if (!isLoggedIn(curUser))
    return User();

  auto users = userDB.find(username);
  if (users.empty())
    return User();

  if (curUser != username && getPrivilege(curUser) <= users[0].privilege)
    return User();

  return users[0];
}

User UserManager::modifyProfile(const string32 &curUser,
                                const string32 &username,
                                const string32 &password, const string32 &name,
                                const string32 &mailAddr, int privilege) {
  if (!isLoggedIn(curUser))
    return User();

  auto users = userDB.find(username);
  if (users.empty())
    return User();

  if (curUser != username && getPrivilege(curUser) <= users[0].privilege)
    return User();

  if (privilege != -1 && getPrivilege(curUser) <= privilege)
    return User();

  User modified = users[0];
  if (!password.empty())
    modified.password = password;
  if (!name.empty())
    modified.name = name;
  if (!mailAddr.empty())
    modified.mailAddr = mailAddr;
  if (privilege != -1)
    modified.privilege = privilege;

  userDB.remove(username, users[0]);
  userDB.insert(username, modified);
  return modified;
}

void UserManager::clean() {
  loggedInUsers.clear();
  userDB.clear();
}

void UserManager::clearLoggedInUsers() { loggedInUsers.clear(); }

bool UserManager::isLoggedIn(const string32 &username) const {
  return loggedInUsers.find(username) != loggedInUsers.cend();
}

int UserManager::getPrivilege(const string32 &username) const {
  auto it = loggedInUsers.find(username);
  if (it != loggedInUsers.cend())
    return it->second;
  return -1;
}

bool UserManager::isFirstUser() const { return userDB.isEmpty; }

#endif // USER_MANAGER_HPP