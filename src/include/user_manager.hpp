#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include <string>
#include <map>
#include <memory>
#include <functional>
#include "database.hpp"

class Session; // Forward declaration

class UserManager {
public:
    UserManager(Database& db);
    ~UserManager() = default;

    // User authentication and registration
    bool registerUser(const std::string& username, const std::string& email, const std::string& password);
    bool authenticateUser(const std::string& username, const std::string& password, int& user_id);
    
    // Session management
    void addSession(int user_id, const std::string& username, std::shared_ptr<Session> session);
    void removeSession(int user_id);
    bool isSessionActive(int user_id);
    std::shared_ptr<Session> getSession(int user_id);
    std::shared_ptr<Session> getSessionByUsername(const std::string& username);
    
    // User information
    std::unique_ptr<User> getUser(const std::string& username);
    std::unique_ptr<User> getUserById(int user_id);

private:
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    
    Database& db_;
    std::map<int, std::shared_ptr<Session>> active_sessions_;
    std::map<int, std::string> user_id_to_username_;
};

#endif // USER_MANAGER_HPP
