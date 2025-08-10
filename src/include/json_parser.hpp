#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "user_manager.hpp"
#include "router.hpp"

class Session; // Forward declaration

class JsonParser {
public:
    JsonParser(UserManager& user_manager, Router& router);
    ~JsonParser() = default;

    // Message parsing and handling
    void parseMessage(const std::string& raw_message, std::shared_ptr<Session> session);
    
private:
    void handleRequest(const nlohmann::json& message, std::shared_ptr<Session> session);
    
    // Command handlers
    void handleRegister(const nlohmann::json& message, std::shared_ptr<Session> session);
    void handleLogin(const nlohmann::json& message, std::shared_ptr<Session> session);
    void handleMessage(const nlohmann::json& message, std::shared_ptr<Session> session);
    void handleTyping(const nlohmann::json& message, std::shared_ptr<Session> session);
    
    // Helper methods
    bool isSessionAuthenticated(std::shared_ptr<Session> session);
    void sendResponse(std::shared_ptr<Session> session, const std::string& type, bool success, const std::string& message = "");
    
    UserManager& user_manager_;
    Router& router_;
};

#endif // JSON_PARSER_HPP
