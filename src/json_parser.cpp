#include "include/json_parser.hpp"
#include "include/session.hpp"
#include <iostream>
#include <ctime>

using json = nlohmann::json;

JsonParser::JsonParser(UserManager& user_manager, Router& router)
    : user_manager_(user_manager), router_(router) {}

void JsonParser::parseMessage(const std::string& raw_message, std::shared_ptr<Session> session) {
    try {
        json message = json::parse(raw_message);
        std::cout << "Received message: " << message.dump() << std::endl;
        
        handleRequest(message, session);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        sendResponse(session, "error", false, "Invalid JSON format");
    } catch (const std::exception& e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
        sendResponse(session, "error", false, "Internal server error");
    }
}

void JsonParser::handleRequest(const json& message, std::shared_ptr<Session> session) {
    try {
        if (!message.contains("type")) {
            sendResponse(session, "error", false, "Missing message type");
            return;
        }
        
        std::string type = message["type"];
        
        if (type == "register") {
            handleRegister(message, session);
        } else if (type == "login") {
            handleLogin(message, session);
        } else if (type == "message") {
            handleMessage(message, session);
        } else if (type == "typing") {
            handleTyping(message, session);
        } else {
            sendResponse(session, "error", false, "Unknown message type: " + type);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling request: " << e.what() << std::endl;
        sendResponse(session, "error", false, "Error processing request");
    }
}

void JsonParser::handleRegister(const json& message, std::shared_ptr<Session> session) {
    try {
        if (!message.contains("username") || !message.contains("email") || !message.contains("password")) {
            sendResponse(session, "register", false, "Missing required fields");
            return;
        }
        
        std::string username = message["username"];
        std::string email = message["email"];
        std::string password = message["password"];
        
        bool success = user_manager_.registerUser(username, email, password);
        
        if (success) {
            sendResponse(session, "register", true, "Registration successful");
        } else {
            sendResponse(session, "register", false, "Registration failed - user may already exist");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in handleRegister: " << e.what() << std::endl;
        sendResponse(session, "register", false, "Registration error");
    }
}

void JsonParser::handleLogin(const json& message, std::shared_ptr<Session> session) {
    try {
        if (!message.contains("username") || !message.contains("password")) {
            sendResponse(session, "login", false, "Missing username or password");
            return;
        }
        
        std::string username = message["username"];
        std::string password = message["password"];
        int user_id;
        
        bool success = user_manager_.authenticateUser(username, password, user_id);
        
        if (success) {
            // Устанавливаем аутентификацию сессии
            session->setAuthenticated(user_id, username);
            
            // Добавляем сессию в UserManager
            user_manager_.addSession(user_id, username, session);
            
            sendResponse(session, "login", true, "Login successful");
            
            // TODO: Отправить оффлайн сообщения
            // router_.sendStoredMessages(user_id, session);
        } else {
            sendResponse(session, "login", false, "Invalid username or password");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in handleLogin: " << e.what() << std::endl;
        sendResponse(session, "login", false, "Login error");
    }
}

void JsonParser::handleMessage(const json& message, std::shared_ptr<Session> session) {
    try {
        if (!isSessionAuthenticated(session)) {
            sendResponse(session, "message", false, "Not authenticated");
            return;
        }
        
        if (!message.contains("to") || !message.contains("content")) {
            sendResponse(session, "message", false, "Missing recipient or content");
            return;
        }
        
        // Передаем сообщение в Router для маршрутизации
        router_.routeMessage(message, session, session->getUserId());
        
        sendResponse(session, "message", true, "Message sent");
    } catch (const std::exception& e) {
        std::cerr << "Error in handleMessage: " << e.what() << std::endl;
        sendResponse(session, "message", false, "Message delivery error");
    }
}

void JsonParser::handleTyping(const json& message, std::shared_ptr<Session> session) {
    try {
        if (!isSessionAuthenticated(session)) {
            return; // Игнорируем typing от неаутентифицированных пользователей
        }
        
        if (!message.contains("to") || !message.contains("is_typing")) {
            return; // Игнорируем некорректные typing сообщения
        }
        
        std::string to_username = message["to"];
        bool is_typing = message["is_typing"];
        
        router_.sendTypingStatus(session->getUsername(), to_username, is_typing);
    } catch (const std::exception& e) {
        std::cerr << "Error in handleTyping: " << e.what() << std::endl;
    }
}

bool JsonParser::isSessionAuthenticated(std::shared_ptr<Session> session) {
    if (!session->isAuthenticated()) {
        return false;
    }
    
    // Дополнительная проверка через UserManager
    return user_manager_.isSessionActive(session->getUserId());
}

void JsonParser::sendResponse(std::shared_ptr<Session> session, const std::string& type, bool success, const std::string& message) {
    try {
        json response;
        response["type"] = type + "_response";
        response["success"] = success;
        response["message"] = message;
        response["timestamp"] = std::time(nullptr);
        
        session->send(response);
    } catch (const std::exception& e) {
        std::cerr << "Error sending response: " << e.what() << std::endl;
    }
}
