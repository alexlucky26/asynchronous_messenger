#ifndef CLIENT_STATE_MACHINE_H
#define CLIENT_STATE_MACHINE_H

#include <string>
#include <memory>
#include <functional>

// Forward declarations
class ClientConnection;
class MessageReceiver;

enum class ClientState {
    TryConnect,
    Disconnected,
    Connected,
    AwaitingLogin,
    Registered,
    LoggedIn,
    Menu,
    ChatSelected,
    Chatting
};

class ClientStateMachine {
public:
    using StateChangeCallback = std::function<void(ClientState, ClientState)>;

    ClientStateMachine(std::shared_ptr<ClientConnection> connection);
    ~ClientStateMachine();

    // State management
    void transitionToState(ClientState new_state);
    ClientState getCurrentState() const { return current_state_; }
    std::string getStateString() const;

    // State-specific actions
    bool tryConnect(const std::string& host, const std::string& port);
    bool registerUser(const std::string& username, const std::string& email, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);
    void startChat(const std::string& target_username);
    void sendMessage(const std::string& content);
    void sendTypingStatus(bool is_typing);
    void exitChat();
    void logout();

    // User info
    const std::string& getCurrentUsername() const { return current_username_; }
    const std::string& getChatTarget() const { return chat_target_; }

    // Callbacks
    void setStateChangeCallback(StateChangeCallback callback) { state_change_callback_ = callback; }

    // Initialize message receiver (call after construction)
    void setMessageReceiver(std::shared_ptr<MessageReceiver> message_receiver) {
        message_receiver_ = message_receiver;
    }

private:
    void handleConnectionLost();
    
    ClientState current_state_;
    std::shared_ptr<ClientConnection> connection_;
    std::shared_ptr<MessageReceiver> message_receiver_;
    
    // User session data
    std::string current_username_;
    std::string chat_target_;
    
    // Callbacks
    StateChangeCallback state_change_callback_;
};

#endif // CLIENT_STATE_MACHINE_H
