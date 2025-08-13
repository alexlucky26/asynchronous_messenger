#ifndef ROUTER_H
#define ROUTER_H

#include <memory>
#include <nlohmann/json.hpp>
#include "database.h"
#include "user_manager.h"

class Session; // Forward declaration

class Router {
public:
    Router(Database& db, UserManager& user_manager);
    ~Router() = default;

    // Message routing
    void routeMessage(const nlohmann::json& message, std::shared_ptr<Session> sender_session, int sender_user_id);
    void sendTypingStatus(const std::string& from_username, const std::string& to_username, bool is_typing);
    
    // File operations (for future implementation)
    void routeFileTransfer(const nlohmann::json& message, std::shared_ptr<Session> sender_session);

    void sendStoredMessages(int user_id, std::shared_ptr<Session> session);
private:
    void deliverMessage(const nlohmann::json& message, int sender_id, const std::string& receiver_username);
    //void storeOfflineMessage(const nlohmann::json& message, int sender_id, int receiver_id);
    
    Database& db_;
    UserManager& user_manager_;
};

#endif // ROUTER_HPP
