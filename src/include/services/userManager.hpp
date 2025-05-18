#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include "storage/bptStorage.hpp"
#include "stl/map.hpp"
#include <string>

using sjtu::map;

struct User {
    std::string username;
    std::string password;
    std::string name;
    std::string mailAddr;
    int privilege;
    
    User() = default;
    User(const std::string& un, const std::string& pw, const std::string& nm, 
         const std::string& ma, int priv)
        : username(un), password(pw), name(nm), mailAddr(ma), privilege(priv) {}
};

class UserManager {
private:
    BPTStorage<std::string, User> userDB;
    map<std::string, int> loggedInUsers; // username -> privilege
    
public:
    UserManager(const std::string& filename);
    
    bool addUser(const std::string& curUser, const std::string& username, 
                const std::string& password, const std::string& name,
                const std::string& mailAddr, int privilege);
    
    bool login(const std::string& username, const std::string& password);
    bool logout(const std::string& username);
    
    User queryProfile(const std::string& curUser, const std::string& username);
    bool modifyProfile(const std::string& curUser, const std::string& username,
                      const std::string& password, const std::string& name,
                      const std::string& mailAddr, int privilege);
    
    bool isLoggedIn(const std::string& username) const;
    int getPrivilege(const std::string& username) const;
    
    bool isFirstUser() const;
};

#endif // USER_MANAGER_HPP