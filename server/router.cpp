#include "include/router.h"
#include "include/session.h"
#include <iostream>
#include <ctime>

using json = nlohmann::json;

Router::Router(Database& db, UserManager& user_manager) 
    : db_(db), user_manager_(user_manager) {}

void Router::routeMessage(const json& message, std::shared_ptr<Session> sender_session, int sender_user_id) {
    try {
        std::string message_type = message["type"];
        
        if (message_type == "message") {
            std::string receiver_username = message["to"];
            deliverMessage(message, sender_user_id, receiver_username);
        } else {
            std::cerr << "Unknown message type: " << message_type << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error routing message: " << e.what() << std::endl;
    }
}

void Router::sendTypingStatus(const std::string& from_username, const std::string& to_username, bool is_typing) {
    try {
        auto receiver_session = user_manager_.getSessionByUsername(to_username);
        
        if (receiver_session != nullptr) {
            json typing_message;
            typing_message["type"] = "typing";
            typing_message["from"] = from_username;
            typing_message["is_typing"] = is_typing;
            typing_message["timestamp"] = std::time(nullptr);
            
            receiver_session->send(typing_message);
            std::cout << "Typing status sent from " << from_username << " to " << to_username << std::endl;
        } else {
            std::cout << "User " << to_username << " is offline, typing status not sent" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error sending typing status: " << e.what() << std::endl;
    }
}

void Router::deliverMessage(const json& message, int sender_id, const std::string& receiver_username) {
    try {
        // Получаем информацию о получателе
        auto receiver_user = user_manager_.getUser(receiver_username);
        if (receiver_user == nullptr) {
            std::cout << "Receiver " << receiver_username << " not found!" << std::endl;
            return;
        }
        
        int receiver_id = receiver_user->id;
        std::string content = message["content"];
        
        // Сохраняем сообщение в базу данных
        bool stored = db_.storeMessage(sender_id, receiver_id, content);
        if (!stored) {
            std::cerr << "Failed to store message in database" << std::endl;
            return;
        }
        
        // Проверяем, онлайн ли получатель
        auto receiver_session = user_manager_.getSession(receiver_id);
        
        if (receiver_session != nullptr) {
            // Пользователь онлайн - отправляем сообщение сразу
            json delivery_message = message;
            delivery_message["timestamp"] = std::time(nullptr);
            delivery_message["delivered"] = true;
            
            receiver_session->send(delivery_message);
            std::cout << "Message delivered to " << receiver_username << " (online)" << std::endl;
        } else {
            // Пользователь оффлайн - сообщение уже сохранено в БД
            std::cout << "Message stored for offline user: " << receiver_username << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error delivering message: " << e.what() << std::endl;
    }
}

void Router::storeOfflineMessage(const json& message, int sender_id, int receiver_id) {
    try {
        std::string content = message["content"];
        bool stored = db_.storeMessage(sender_id, receiver_id, content);
        
        if (stored) {
            std::cout << "Offline message stored for user ID: " << receiver_id << std::endl;
        } else {
            std::cerr << "Failed to store offline message" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error storing offline message: " << e.what() << std::endl;
    }
}

void Router::sendStoredMessages(int user_id, std::shared_ptr<Session> session) {
    try {
        // Получаем все оффлайн-сообщения для пользователя
        std::vector<Message> offline_messages = db_.getOfflineMessages(user_id);
        
        if (!offline_messages.empty()) {
            std::cout << "Sending " << offline_messages.size() << " stored messages to user ID: " << user_id << std::endl;
            
            for (const auto& msg : offline_messages) {
                // Получаем информацию об отправителе
                auto sender_user = user_manager_.getUserById(msg.sender_id);
                
                json stored_message;
                stored_message["type"] = "message";
                stored_message["from"] = sender_user ? sender_user->username : "unknown";
                stored_message["content"] = msg.content;
                stored_message["timestamp"] = msg.sent_at;
                stored_message["stored"] = true;
                
                if (msg.is_file) {
                    stored_message["is_file"] = true;
                    stored_message["file_path"] = msg.file_path;
                }
                
                session->send(stored_message);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error sending stored messages: " << e.what() << std::endl;
    }
}

void Router::routeFileTransfer(const json& message, std::shared_ptr<Session> sender_session) {
    // Placeholder for future file transfer implementation
    std::cout << "File transfer not yet implemented" << std::endl;
}
