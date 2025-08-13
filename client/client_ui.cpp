#include "client_ui.h"
#include "client_state_machine.h"
#include "message_receiver.h"
#include "message_formatter.h"
#include <iostream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

ClientUI::ClientUI(std::shared_ptr<ClientStateMachine> state_machine, 
                   std::shared_ptr<MessageReceiver> message_receiver)
    : state_machine_(state_machine), message_receiver_(message_receiver),
      formatter_(std::make_unique<MessageFormatter>()),
      running_(false), in_chat_mode_(false), chat_thread_running_(false) {
    
    setupMessageCallbacks();
}

ClientUI::~ClientUI() {
    shutdown();
}

void ClientUI::run() {
    running_ = true;
    
    displayWelcomeScreen();
    
    while (running_) {
        handleUserInput();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ClientUI::shutdown() {
    running_ = false;
    stopChatThread();
    
    if (state_machine_) {
        state_machine_->logout();
    }
}

void ClientUI::displayWelcomeScreen() {
    clearScreen();
    std::cout << formatter_->createSeparator('=', 60) << std::endl;
    std::cout << formatter_->centerText("🔗 ASYNCHRONOUS MESSENGER CLIENT", 60) << std::endl;
    std::cout << formatter_->centerText("Добро пожаловать!", 60) << std::endl;
    std::cout << formatter_->createSeparator('=', 60) << std::endl;
    std::cout << std::endl;
}

void ClientUI::displayLoginMenu() {
    std::cout << "\n" << formatter_->createSeparator('-', 40) << std::endl;
    std::cout << "Выберите действие:" << std::endl;
    std::cout << "[1] Войти в аккаунт" << std::endl;
    std::cout << "[2] Зарегистрироваться" << std::endl;
    std::cout << "[3] Выйти" << std::endl;
    std::cout << formatter_->createSeparator('-', 40) << std::endl;
}

void ClientUI::displayMainMenu() {
    std::cout << "\n" << formatter_->createSeparator('-', 50) << std::endl;
    std::cout << "Добро пожаловать, " << state_machine_->getCurrentUsername() << "!" << std::endl;
    std::cout << "[1] Посмотреть список пользователей" << std::endl;
    std::cout << "[2] Начать чат" << std::endl;
    std::cout << "[3] Выйти из аккаунта" << std::endl;
    std::cout << formatter_->createSeparator('-', 50) << std::endl;
}

void ClientUI::handleUserInput() {
    ClientState current_state = state_machine_->getCurrentState();
    
    switch (current_state) {
        case ClientState::Disconnected:
            // Try to connect
            if (!state_machine_->tryConnect("127.0.0.1", "9999")) {
                displayError("Не удалось подключиться к серверу");
                running_ = false;
            }
            break;
            
        case ClientState::AwaitingLogin:
            displayLoginMenu();
            if (!handleLoginMenu()) {
                running_ = false;
            }
            break;
            
        case ClientState::Menu:
            displayMainMenu();
            if (!handleMainMenu()) {
                running_ = false;
            }
            break;
            
        case ClientState::Chatting:
            // Chat input is handled in separate thread
            if (!in_chat_mode_) {
                enterChatMode(state_machine_->getChatTarget());
            }
            break;
            
        default:
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            break;
    }
}

bool ClientUI::handleLoginMenu() {
    std::string input = getInput("Ваш выбор");
    
    if (input == "1") {
        return performLogin();
    } else if (input == "2") {
        return performRegistration();
    } else if (input == "3") {
        return false; // Exit
    } else {
        displayError("Неверный выбор. Попробуйте снова.");
        return true;
    }
}

bool ClientUI::handleMainMenu() {
    std::string input = getInput("Ваш выбор");
    
    if (input == "1") {
        displaySystemMessage("Список пользователей (функция в разработке)");
        return true;
    } else if (input == "2") {
        std::string target = getInput("Введите имя пользователя для чата");
        if (!target.empty()) {
            state_machine_->startChat(target);
        }
        return true;
    } else if (input == "3") {
        state_machine_->logout();
        return false;
    } else {
        displayError("Неверный выбор. Попробуйте снова.");
        return true;
    }
}

bool ClientUI::performLogin() {
    std::cout << "\n=== ВХОД В СИСТЕМУ ===" << std::endl;
    
    std::string username = getInput("Имя пользователя");
    std::string password = getPassword("Пароль");
    
    if (username.empty() || password.empty()) {
        displayError("Имя пользователя и пароль не могут быть пустыми");
        return true;
    }
    
    std::cout << "Выполняется вход..." << std::endl;
    state_machine_->loginUser(username, password);
    
    // Wait for server response (handled by MessageReceiver)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return true;
}

bool ClientUI::performRegistration() {
    std::cout << "\n=== РЕГИСТРАЦИЯ ===" << std::endl;
    
    std::string username = getInput("Имя пользователя");
    std::string email = getInput("Email");
    std::string password = getPassword("Пароль");
    
    if (username.empty() || email.empty() || password.empty()) {
        displayError("Все поля должны быть заполнены");
        return true;
    }
    
    std::cout << "Выполняется регистрация..." << std::endl;
    state_machine_->registerUser(username, email, password);
    
    // Wait for server response (handled by MessageReceiver)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return true;
}

void ClientUI::enterChatMode(const std::string& target_username) {
    if (in_chat_mode_) {
        return;
    }
    
    in_chat_mode_ = true;
    current_chat_target_ = target_username;
    
    displayChatInterface(target_username);
    startChatThread();
}

void ClientUI::exitChatMode() {
    if (!in_chat_mode_) {
        return;
    }
    
    in_chat_mode_ = false;
    stopChatThread();
    current_chat_target_.clear();
    current_typing_status_.clear();
    
    state_machine_->exitChat();
}

void ClientUI::displayChatInterface(const std::string& target_username) {
    clearScreen();
    std::cout << formatter_->createSeparator('=', 60) << std::endl;
    std::cout << formatter_->centerText("ЧАТ С " + target_username, 60) << std::endl;
    std::cout << formatter_->createSeparator('=', 60) << std::endl;
    std::cout << "Введите сообщение или /exit для выхода из чата" << std::endl;
    std::cout << formatter_->createSeparator('-', 60) << std::endl;
}

void ClientUI::startChatThread() {
    if (chat_thread_running_) {
        return;
    }
    
    chat_thread_running_ = true;
    chat_thread_ = std::make_unique<std::thread>([this]() {
        while (chat_thread_running_ && in_chat_mode_) {
            handleChatInput();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void ClientUI::stopChatThread() {
    chat_thread_running_ = false;
    
    if (chat_thread_ && chat_thread_->joinable()) {
        chat_thread_->join();
    }
    chat_thread_.reset();
}

void ClientUI::handleChatInput() {
    std::cout << "> ";
    std::string input;
    std::getline(std::cin, input);
    
    if (input == "/exit") {
        exitChatMode();
        return;
    }
    
    if (!input.empty()) {
        state_machine_->sendMessage(input);
        
        // Display own message immediately
        displayMessage(state_machine_->getCurrentUsername(), input, std::time(nullptr));
    }
}

void ClientUI::displayMessage(const std::string& sender, const std::string& content, std::time_t timestamp) {
    bool is_self = (sender == state_machine_->getCurrentUsername());
    std::string formatted = formatter_->formatChatMessage(sender, content, timestamp);
    
    std::cout << formatted << std::endl;
}

void ClientUI::displayTypingStatus(const std::string& username, bool is_typing) {
    if (is_typing) {
        current_typing_status_ = formatter_->formatTypingStatus(username, true);
        std::cout << current_typing_status_ << std::endl;
    } else {
        current_typing_status_.clear();
        // Clear the typing status line
        std::cout << "\r                                    \r";
    }
}

void ClientUI::displaySystemMessage(const std::string& message) {
    std::cout << formatter_->formatSystemMessage(message) << std::endl;
}

void ClientUI::setupMessageCallbacks() {
    // Set up message callback
    message_receiver_->setMessageCallback([this](const nlohmann::json& message) {
        if (message.contains("from") && message.contains("content")) {
            std::string from = message["from"];
            std::string content = message["content"];
            std::time_t timestamp = message.value("timestamp", std::time(nullptr));
            
            if (in_chat_mode_ && from == current_chat_target_) {
                displayMessage(from, content, timestamp);
            }
        }
    });
    
    // Set up typing callback
    message_receiver_->setTypingCallback([this](const std::string& username, bool is_typing) {
        if (in_chat_mode_ && username == current_chat_target_) {
            displayTypingStatus(username, is_typing);
        }
    });
    
    // Set up error callback
    message_receiver_->setErrorCallback([this](const std::string& error) {
        displayError(error);
    });
}

void ClientUI::onStateChanged(ClientState old_state, ClientState new_state) {
    // Handle state changes if needed
}

std::string ClientUI::getInput(const std::string& prompt) {
    std::cout << formatter_->formatPrompt(prompt);
    std::string input;
    std::getline(std::cin, input);
    return input;
}

std::string ClientUI::getPassword(const std::string& prompt) {
    std::cout << formatter_->formatPrompt(prompt);
    std::string password;
    
#ifdef _WIN32
    // Windows password input
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << std::endl;
#else
    // Unix password input
    termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;
    new_settings.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    
    std::getline(std::cin, password);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    std::cout << std::endl;
#endif
    
    return password;
}

void ClientUI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ClientUI::displayHeader(const std::string& title) {
    std::cout << formatter_->createSeparator('=', 60) << std::endl;
    std::cout << formatter_->centerText(title, 60) << std::endl;
    std::cout << formatter_->createSeparator('=', 60) << std::endl;
}

void ClientUI::displayError(const std::string& error) {
    std::cout << formatter_->formatError(error) << std::endl;
}

void ClientUI::displaySuccess(const std::string& message) {
    std::cout << formatter_->formatSuccess(message) << std::endl;
}
