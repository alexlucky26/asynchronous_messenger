#include "message_receiver.h"
#include "client_connection.h"
#include "client_state_machine.h"
#include <iostream>

using json = nlohmann::json;

MessageReceiver::MessageReceiver(std::shared_ptr<ClientConnection> connection, 
                                std::shared_ptr<ClientStateMachine> state_machine)
    : connection_(connection), state_machine_(state_machine), listening_(false) {}

MessageReceiver::~MessageReceiver() {
    stopListening();
}

void MessageReceiver::startListening() {
    if (listening_) {
        return;
    }
    
    listening_ = true;
    std::cout << "MessageReceiver: Started listening for incoming messages" << std::endl;
    
    // Set up callback for incoming messages
    connection_->setMessageCallback([this](const nlohmann::json& message) {
        handleIncomingMessage(message);
    });
    
    connection_->setErrorCallback([this](const std::string& error) {
        if (error_callback_) {
            error_callback_(error);
        }
        listening_ = false;
    });
}

void MessageReceiver::stopListening() {
    if (!listening_) {
        return;
    }
    
    listening_ = false;
    std::cout << "MessageReceiver: Stopped listening" << std::endl;
    
    // Clear callbacks
    connection_->setMessageCallback(nullptr);
    connection_->setErrorCallback(nullptr);
}

void MessageReceiver::handleIncomingMessage(const nlohmann::json& message) {
    try {
        if (!message.contains("type")) {
            std::cerr << "Received message without type field" << std::endl;
            return;
        }
        
        std::string type = message["type"];
        std::cout << "MessageReceiver: Processing message type: " << type << std::endl;
        
        if (type.length() > 9 && type.substr(type.length() - 9) == "_response") {
            // Server responses (login_response, register_response, message_response)
            handleServerResponse(message);
        } else if (type == "message") {
            // Chat messages from other users
            handleChatMessage(message);
        } else if (type == "typing") {
            // Typing status updates
            handleTypingStatus(message);
        } else if (type == "error") {
            // Error messages
            handleError(message);
        } else {
            std::cout << "MessageReceiver: Unknown message type: " << type << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "MessageReceiver error: " << e.what() << std::endl;
    }
}

void MessageReceiver::handleServerResponse(const nlohmann::json& response) {
    std::string type = response["type"];
    bool success = response.value("success", false);
    std::string message = response.value("message", "");
    
    std::cout << "Server response - Type: " << type << ", Success: " << success << ", Message: " << message << std::endl;
    
    if (type == "login_response") {
        if (success) {
            state_machine_->transitionToState(ClientState::LoggedIn);
            state_machine_->transitionToState(ClientState::Menu);
            std::cout << "✅ " << message << std::endl;
        } else {
            std::cout << "❌ Login failed: " << message << std::endl;
            // Stay in AwaitingLogin state
        }
    } else if (type == "register_response") {
        if (success) {
            state_machine_->transitionToState(ClientState::Registered);
            // НЕ переходим в LoggedIn - пользователь должен залогиниться отдельно
            std::cout << "✅ " << message << std::endl;
            std::cout << "Теперь войдите в систему с вашими данными." << std::endl;
            // Возвращаемся в AwaitingLogin для логина
            state_machine_->transitionToState(ClientState::AwaitingLogin);
        } else {
            std::cout << "❌ Registration failed: " << message << std::endl;
            // Stay in AwaitingLogin state
        }
    } else if (type == "message_response") {
        if (success) {
            std::cout << "✅ Message sent successfully" << std::endl;
        } else {
            std::cout << "❌ Message failed to send: " << message << std::endl;
        }
    }
}

void MessageReceiver::handleChatMessage(const nlohmann::json& message) {
    if (message.contains("from") && message.contains("content")) {
        std::string from = message["from"];
        std::string content = message["content"];
        std::time_t timestamp = message.value("timestamp", std::time(nullptr));
        
        if (message_callback_) {
            message_callback_(message);
        } else {
            // Default display if no callback set
            std::cout << "\n📨 [" << from << "]: " << content << std::endl;
            std::cout << "> "; // Show prompt again
            std::cout.flush();
        }
    }
}

void MessageReceiver::handleTypingStatus(const nlohmann::json& message) {
    if (message.contains("from") && message.contains("is_typing")) {
        std::string from = message["from"];
        bool is_typing = message["is_typing"];
        
        if (typing_callback_) {
            typing_callback_(from, is_typing);
        } else {
            // Default display if no callback set
            if (is_typing) {
                std::cout << "\n💬 " << from << " печатает..." << std::endl;
            } else {
                // Clear typing status (could be implemented as clearing a status line)
                std::cout << "\r                                    \r"; // Clear line
            }
            std::cout << "> ";
            std::cout.flush();
        }
    }
}

void MessageReceiver::handleError(const nlohmann::json& error) {
    std::string message = error.value("message", "Unknown error");
    std::cerr << "❌ Server error: " << message << std::endl;
    
    if (error_callback_) {
        error_callback_(message);
    }
}
