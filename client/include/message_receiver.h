#ifndef MESSAGE_RECEIVER_H
#define MESSAGE_RECEIVER_H

#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

// Forward declarations
class ClientConnection;
class ClientStateMachine;

class MessageReceiver {
public:
    using MessageCallback = std::function<void(const nlohmann::json&)>;
    using TypingCallback = std::function<void(const std::string&, bool)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    MessageReceiver(std::shared_ptr<ClientConnection> connection, 
                   std::shared_ptr<ClientStateMachine> state_machine);
    ~MessageReceiver();

    // Listening control
    void startListening();
    void stopListening();
    bool isListening() const { return listening_; }

    // Callbacks for different message types
    void setMessageCallback(MessageCallback callback) { message_callback_ = callback; }
    void setTypingCallback(TypingCallback callback) { typing_callback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }

private:
    void handleIncomingMessage(const nlohmann::json& message);
    void handleServerResponse(const nlohmann::json& response);
    void handleChatMessage(const nlohmann::json& message);
    void handleTypingStatus(const nlohmann::json& message);
    void handleError(const nlohmann::json& error);

    std::shared_ptr<ClientConnection> connection_;
    std::shared_ptr<ClientStateMachine> state_machine_;
    
    bool listening_;
    
    // Callbacks
    MessageCallback message_callback_;
    TypingCallback typing_callback_;
    ErrorCallback error_callback_;
};

#endif // MESSAGE_RECEIVER_H
