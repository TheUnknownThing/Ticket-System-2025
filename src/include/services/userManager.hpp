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
};

class UserManager {
private:
    BPTStorage<string64, User> userDB;
    map<string64, int> loggedInUsers; // username -> privilege
    
public:
    UserManager(const string64& filename);
    
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

#endif // USER_MANAGER_HPP