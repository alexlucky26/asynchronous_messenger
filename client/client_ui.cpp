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
#include <sys/select.h>
#include <fcntl.h>
#endif

ClientUI::ClientUI(std::shared_ptr<ClientStateMachine> state_machine, 
                   std::shared_ptr<MessageReceiver> message_receiver)
    : state_machine_(state_machine), message_receiver_(message_receiver),
      formatter_(std::make_unique<MessageFormatter>()),
      running_(false), in_chat_mode_(false), chat_thread_running_(false),
      typing_cleanup_running_(false) {
    
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
    stopTypingCleanupThread();
    
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
    startTypingCleanupThread();
}

void ClientUI::exitChatMode() {
    if (!in_chat_mode_) {
        return;
    }
    
    in_chat_mode_ = false;
    stopChatThread();
    stopTypingCleanupThread();
    current_chat_target_.clear();
    current_typing_status_.clear();
    current_typing_user_.clear();
    
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
    bool is_typing = false;
    auto last_keypress = std::chrono::steady_clock::now();
    
    // Таймер для проверки статуса typing
    std::thread typing_timer([&]() {
        while (chat_thread_running_ && in_chat_mode_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_keypress);
            
            // Если прошло больше 1 секунды без ввода и мы "печатаем" - сбрасываем статус
            if (elapsed > std::chrono::milliseconds(1000) && is_typing) {
                is_typing = false;
                state_machine_->sendTypingStatus(false);
            }
        }
    });
    
#ifdef _WIN32
    // Windows: посимвольное чтение
    char ch;
    while (chat_thread_running_ && in_chat_mode_) {
        if (_kbhit()) {
            ch = _getch();
            
            if (ch == '\r') { // Enter
                break;
            } else if (ch == '\b') { // Backspace
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b";
                }
            } else if (ch >= 32 && ch <= 126) { // Печатные символы
                input.push_back(ch);
                std::cout << ch;
                
                // Отправляем статус "печатает" при первом символе
                if (!is_typing && !input.empty()) {
                    is_typing = true;
                    state_machine_->sendTypingStatus(true);
                }
            }
            
            // Обновляем время последнего нажатия клавиши
            last_keypress = std::chrono::steady_clock::now();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
#else
    // Unix: посимвольное чтение с termios
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO); // Отключаем канонический режим и эхо
    new_termios.c_cc[VMIN] = 0;  // Неблокирующее чтение
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    char ch;
    fd_set readfds;
    struct timeval timeout;
    
    while (chat_thread_running_ && in_chat_mode_) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; // 10ms timeout
        
        int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        
        if (result > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == '\n' || ch == '\r') { // Enter
                    break;
                } else if (ch == 127 || ch == '\b') { // Backspace/Delete
                    if (!input.empty()) {
                        input.pop_back();
                        std::cout << "\b \b" << std::flush;
                    }
                } else if (ch >= 32 && ch <= 126) { // Печатные символы
                    input.push_back(ch);
                    std::cout << ch << std::flush;
                    
                    // Отправляем статус "печатает" при первом символе
                    if (!is_typing && !input.empty()) {
                        is_typing = true;
                        state_machine_->sendTypingStatus(true);
                    }
                }
                
                // Обновляем время последнего нажатия клавиши
                last_keypress = std::chrono::steady_clock::now();
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Восстанавливаем настройки терминала
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
#endif
    
    // Завершаем таймер
    typing_timer.detach();
    
    // Сбрасываем статус "печатает" после завершения ввода
    if (is_typing) {
        state_machine_->sendTypingStatus(false);
    }
    
    std::cout << std::endl;
    
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
        // Сначала очищаем предыдущий статус если есть
        if (!current_typing_status_.empty()) {
            std::cout << "\r" << std::string(current_typing_status_.length() + 10, ' ') << "\r";
        }
        
        current_typing_status_ = formatter_->formatTypingStatus(username, true);
        current_typing_user_ = username;
        last_typing_time_ = std::chrono::steady_clock::now();
        
        std::cout << current_typing_status_ << std::flush;
    } else {
        // Очищаем статус конкретного пользователя
        if (!current_typing_status_.empty() && current_typing_user_ == username) {
            std::cout << "\r" << std::string(current_typing_status_.length() + 10, ' ') << "\r";
            current_typing_status_.clear();
            current_typing_user_.clear();
            
            // Показываем prompt заново
            std::cout << "> " << std::flush;
        }
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

void ClientUI::startTypingCleanupThread() {
    if (typing_cleanup_running_) {
        return;
    }
    
    typing_cleanup_running_ = true;
    typing_cleanup_thread_ = std::make_unique<std::thread>([this]() {
        while (typing_cleanup_running_ && in_chat_mode_) {
            clearTypingStatusAfterDelay();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
}

void ClientUI::stopTypingCleanupThread() {
    typing_cleanup_running_ = false;
    
    if (typing_cleanup_thread_ && typing_cleanup_thread_->joinable()) {
        typing_cleanup_thread_->join();
    }
    typing_cleanup_thread_.reset();
}

void ClientUI::clearTypingStatusAfterDelay() {
    // Если есть статус "печатает" и прошло больше 3 секунд с последнего обновления
    if (!current_typing_status_.empty() && !current_typing_user_.empty()) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_typing_time_);
        
        if (elapsed.count() >= 3) {
            // Очищаем статус
            std::cout << "\r" << std::string(current_typing_status_.length() + 10, ' ') << "\r";
            current_typing_status_.clear();
            current_typing_user_.clear();
            
            // Показываем prompt заново
            std::cout << "> " << std::flush;
        }
    }
}
