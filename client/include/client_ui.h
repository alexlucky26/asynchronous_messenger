#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include "client_state_machine.h" // This will include ClientState enum

// Forward declarations
class MessageReceiver;
class MessageFormatter;

enum class MenuOption {
    Login = 1,
    Register = 2,
    Exit = 3,
    ViewUsers = 1,  // In logged-in menu
    StartChat = 2,
    Logout = 3
};

class ClientUI {
public:
    ClientUI(std::shared_ptr<ClientStateMachine> state_machine, 
             std::shared_ptr<MessageReceiver> message_receiver);
    ~ClientUI();

    // Main UI loop
    void run();
    void shutdown();

    // Menu displays
    void displayWelcomeScreen();
    void displayLoginMenu();
    void displayMainMenu();
    void displayChatInterface(const std::string& target_username);

    // Input handling
    void handleUserInput();
    std::string getInput(const std::string& prompt);
    std::string getPassword(const std::string& prompt);

    // Chat interface
    void enterChatMode(const std::string& target_username);
    void exitChatMode();
    void displayMessage(const std::string& sender, const std::string& content, std::time_t timestamp);
    void displayTypingStatus(const std::string& username, bool is_typing);
    void displaySystemMessage(const std::string& message);

    // State change handling
    void onStateChanged(ClientState old_state, ClientState new_state);

private:
    void clearScreen();
    void displayHeader(const std::string& title);
    void displayError(const std::string& error);
    void displaySuccess(const std::string& message);
    
    // Menu handlers
    bool handleLoginMenu();
    bool handleMainMenu();
    void handleChatInput();

    // Authentication flows
    bool performLogin();
    bool performRegistration();

    // Chat management
    void setupMessageCallbacks();
    void startChatThread();
    void stopChatThread();
    void startTypingCleanupThread();
    void stopTypingCleanupThread();
    void clearTypingStatusAfterDelay();

    std::shared_ptr<ClientStateMachine> state_machine_;
    std::shared_ptr<MessageReceiver> message_receiver_;
    std::unique_ptr<MessageFormatter> formatter_;

    // UI state
    bool running_;
    bool in_chat_mode_;
    std::string current_chat_target_;
    std::string current_typing_status_;
    std::string current_typing_user_;
    std::chrono::steady_clock::time_point last_typing_time_;
    
    // Threading for chat input
    std::unique_ptr<std::thread> chat_thread_;
    bool chat_thread_running_;
    std::unique_ptr<std::thread> typing_cleanup_thread_;
    bool typing_cleanup_running_;
};

#endif // CLIENT_UI_H
