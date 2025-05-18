#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include "storage/bptStorage.hpp"
#include "stl/map.hpp"
#include "utils/string64.hpp"
#include <string>

using sjtu::map;
using sjtu::string64;

struct User {
    string64 username;
    string64 password;
    string64 name;
    string64 mailAddr;
    int privilege;
    
    User() = default;
    User(const string64& un, const string64& pw, const string64& nm, 
         const string64& ma, int priv)
        : username(un), password(pw), name(nm), mailAddr(ma), privilege(priv) {}

    bool operator==(const User& other) const {
        return username == other.username && password == other.password &&
               name == other.name && mailAddr == other.mailAddr &&
               privilege == other.privilege;
    }
    bool operator!=(const User& other) const {
        return !(*this == other);
    }
};

class UserManager {
private:
    BPTStorage<string64, User> userDB;
    map<string64, int> loggedInUsers; // username -> privilege
    
public:
    UserManager(const std::string& filename);
    
    bool addUser(const string64& curUser, const string64& username, 
                const string64& password, const string64& name,
                const string64& mailAddr, int privilege);
    
    bool login(const string64& username, const string64& password);
    bool logout(const string64& username);
    
    User queryProfile(const string64& curUser, const string64& username);
    bool modifyProfile(const string64& curUser, const string64& username,
                      const string64& password, const string64& name,
                      const string64& mailAddr, int privilege);
    
    bool isLoggedIn(const string64& username) const;
    int getPrivilege(const string64& username) const;
    
    bool isFirstUser() const;
};

UserManager::UserManager(const std::string& filename) 
    : userDB(filename + "_user", string64::STRING64_MAX()) {}

bool UserManager::addUser(const string64& curUser, const string64& username, 
                        const string64& password, const string64& name,
                        const string64& mailAddr, int privilege) {
    if (userDB.find(username).size() > 0) return false;
    
    if (!isFirstUser()) {
        if (!isLoggedIn(curUser)) return false;
        if (getPrivilege(curUser) <= privilege) return false;
    } else {
        privilege = 10; // First user gets max privilege
    }
    
    User newUser(username, password, name, mailAddr, privilege);
    userDB.insert(username, newUser);
    return true;
}

bool UserManager::login(const string64& username, const string64& password) {
    auto users = userDB.find(username);
    if (users.empty() || users[0].password != password) return false;
    if (isLoggedIn(username)) return false;
    
    loggedInUsers[username] = users[0].privilege;
    return true;
}

bool UserManager::logout(const string64& username) {
    if (!isLoggedIn(username)) return false;
    loggedInUsers.erase(loggedInUsers.find(username));
    return true;
}

User UserManager::queryProfile(const string64& curUser, const string64& username) {
    if (!isLoggedIn(curUser)) return User();
    
    auto users = userDB.find(username);
    if (users.empty()) return User();
    
    if (curUser != username && getPrivilege(curUser) <= users[0].privilege) 
        return User();
    
    return users[0];
}

bool UserManager::modifyProfile(const string64& curUser, const string64& username,
                              const string64& password, const string64& name,
                              const string64& mailAddr, int privilege) {
    if (!isLoggedIn(curUser)) return false;
    
    auto users = userDB.find(username);
    if (users.empty()) return false;
    
    if (curUser != username && getPrivilege(curUser) <= users[0].privilege)
        return false;
    
    if (privilege != -1 && getPrivilege(curUser) <= privilege)
        return false;
    
    User modified = users[0];
    if (!password.empty()) modified.password = password;
    if (!name.empty()) modified.name = name;
    if (!mailAddr.empty()) modified.mailAddr = mailAddr;
    if (privilege != -1) modified.privilege = privilege;
    
    userDB.remove(username, users[0]);
    userDB.insert(username, modified);
    return true;
}

bool UserManager::isLoggedIn(const string64& username) const {
    return loggedInUsers.find(username) != loggedInUsers.cend();
}

int UserManager::getPrivilege(const string64& username) const {
    auto it = loggedInUsers.find(username);
    if (it != loggedInUsers.cend()) return it->second;
    return -1;
}

bool UserManager::isFirstUser() const {
    return userDB.isEmpty;
}

#endif // USER_MANAGER_HPP