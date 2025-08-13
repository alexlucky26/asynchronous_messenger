#include "client_state_machine.h"
#include "client_connection.h"
#include "message_receiver.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ClientStateMachine::ClientStateMachine(std::shared_ptr<ClientConnection> connection)
    : current_state_(ClientState::Disconnected), connection_(connection) {
    // MessageReceiver will be initialized later when needed
}

ClientStateMachine::~ClientStateMachine() {
    if (message_receiver_) {
        message_receiver_->stopListening();
    }
}

void ClientStateMachine::transitionToState(ClientState new_state) {
    ClientState old_state = current_state_;
    current_state_ = new_state;
    
    std::cout << "State changed from " << getStateString() << " to ";
    current_state_ = new_state;
    std::cout << getStateString() << std::endl;
    
    if (state_change_callback_) {
        state_change_callback_(old_state, new_state);
    }
}

std::string ClientStateMachine::getStateString() const {
    switch (current_state_) {
        case ClientState::TryConnect: return "TryConnect";
        case ClientState::Disconnected: return "Disconnected";
        case ClientState::Connected: return "Connected";
        case ClientState::AwaitingLogin: return "AwaitingLogin";
        case ClientState::Registered: return "Registered";
        case ClientState::LoggedIn: return "LoggedIn";
        case ClientState::Menu: return "Menu";
        case ClientState::ChatSelected: return "ChatSelected";
        case ClientState::Chatting: return "Chatting";
        default: return "Unknown";
    }
}

bool ClientStateMachine::tryConnect(const std::string& host, const std::string& port) {
    transitionToState(ClientState::TryConnect);
    
    bool connected = connection_->connect(host, port);
    
    if (connected) {
        transitionToState(ClientState::Connected);
        connection_->startReceiving();
        message_receiver_->startListening();
        transitionToState(ClientState::AwaitingLogin);
        return true;
    } else {
        transitionToState(ClientState::Disconnected);
        return false;
    }
}

bool ClientStateMachine::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    if (current_state_ != ClientState::AwaitingLogin) {
        std::cerr << "Cannot register in current state: " << getStateString() << std::endl;
        return false;
    }

    json request;
    request["type"] = "register";
    request["username"] = username;
    request["email"] = email;
    request["password"] = password;

    connection_->send(request);
    
    // Note: State transition will happen when we receive server response
    return true;
}

bool ClientStateMachine::loginUser(const std::string& username, const std::string& password) {
    if (current_state_ != ClientState::AwaitingLogin) {
        std::cerr << "Cannot login in current state: " << getStateString() << std::endl;
        return false;
    }

    json request;
    request["type"] = "login";
    request["username"] = username;
    request["password"] = password;

    connection_->send(request);
    current_username_ = username; // Store for later use
    
    // Note: State transition will happen when we receive server response
    return true;
}

void ClientStateMachine::startChat(const std::string& target_username) {
    if (current_state_ != ClientState::Menu) {
        std::cerr << "Cannot start chat in current state: " << getStateString() << std::endl;
        return;
    }

    chat_target_ = target_username;
    transitionToState(ClientState::ChatSelected);
    transitionToState(ClientState::Chatting);
}

void ClientStateMachine::sendMessage(const std::string& content) {
    if (current_state_ != ClientState::Chatting) {
        std::cerr << "Cannot send message in current state: " << getStateString() << std::endl;
        return;
    }

    json message;
    message["type"] = "message";
    message["to"] = chat_target_;
    message["content"] = content;

    connection_->send(message);
}

void ClientStateMachine::sendTypingStatus(bool is_typing) {
    if (current_state_ != ClientState::Chatting) {
        return; // Silently ignore if not in chat
    }

    json typing_msg;
    typing_msg["type"] = "typing";
    typing_msg["to"] = chat_target_;
    typing_msg["is_typing"] = is_typing;

    connection_->send(typing_msg);
}

void ClientStateMachine::exitChat() {
    if (current_state_ == ClientState::Chatting) {
        chat_target_.clear();
        transitionToState(ClientState::Menu);
    }
}

void ClientStateMachine::logout() {
    current_username_.clear();
    chat_target_.clear();
    
    if (message_receiver_) {
        message_receiver_->stopListening();
    }
    
    connection_->disconnect();
    transitionToState(ClientState::Disconnected);
}

void ClientStateMachine::handleConnectionLost() {
    std::cerr << "Connection lost!" << std::endl;
    current_username_.clear();
    chat_target_.clear();
    transitionToState(ClientState::Disconnected);
}
