#include "include/user_manager.hpp"
#include "include/session.hpp"
#include <iostream>
#include <functional>

UserManager::UserManager(Database& db) : db_(db) {}

std::string UserManager::hashPassword(const std::string& password) {
    // Простая реализация хеширования (в реальном проекте используйте bcrypt или подобные)
    std::hash<std::string> hasher;
    size_t hash = hasher(password + "salt_key_2024"); // Добавляем соль
    return std::to_string(hash);
}

bool UserManager::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

bool UserManager::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    try {
        // Проверяем, существует ли пользователь
        auto existing_user = db_.getUser(username);
        if (existing_user != nullptr) {
            std::cout << "User " << username << " already exists!" << std::endl;
            return false;
        }
        
        // Хешируем пароль и сохраняем пользователя
        std::string password_hash = hashPassword(password);
        bool result = db_.addUser(username, email, password_hash);
        
        if (result) {
            std::cout << "User " << username << " registered successfully!" << std::endl;
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error registering user: " << e.what() << std::endl;
        return false;
    }
}

bool UserManager::authenticateUser(const std::string& username, const std::string& password, int& user_id) {
    try {
        auto user = db_.getUser(username);
        if (user == nullptr) {
            std::cout << "User " << username << " not found!" << std::endl;
            return false;
        }
        
        if (verifyPassword(password, user->password_hash)) {
            user_id = user->id;
            std::cout << "User " << username << " authenticated successfully!" << std::endl;
            return true;
        } else {
            std::cout << "Invalid password for user " << username << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error authenticating user: " << e.what() << std::endl;
        return false;
    }
}

void UserManager::addSession(int user_id, const std::string& username, std::shared_ptr<Session> session) {
    // Удаляем предыдущую сессию, если есть
    removeSession(user_id);
    
    active_sessions_[user_id] = session;
    user_id_to_username_[user_id] = username;
    
    std::cout << "Session added for user: " << username << " (ID: " << user_id << ")" << std::endl;
    std::cout << "Total active sessions: " << active_sessions_.size() << std::endl;
}

void UserManager::removeSession(int user_id) {
    auto it = active_sessions_.find(user_id);
    if (it != active_sessions_.end()) {
        std::string username = user_id_to_username_[user_id];
        active_sessions_.erase(it);
        user_id_to_username_.erase(user_id);
        
        std::cout << "Session removed for user: " << username << " (ID: " << user_id << ")" << std::endl;
        std::cout << "Total active sessions: " << active_sessions_.size() << std::endl;
    }
}

bool UserManager::isSessionActive(int user_id) {
    return active_sessions_.find(user_id) != active_sessions_.end();
}

std::shared_ptr<Session> UserManager::getSession(int user_id) {
    auto it = active_sessions_.find(user_id);
    if (it != active_sessions_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Session> UserManager::getSessionByUsername(const std::string& username) {
    // Найдем user_id по username
    for (const auto& pair : user_id_to_username_) {
        if (pair.second == username) {
            return getSession(pair.first);
        }
    }
    return nullptr;
}

std::unique_ptr<User> UserManager::getUser(const std::string& username) {
    return db_.getUser(username);
}

std::unique_ptr<User> UserManager::getUserById(int user_id) {
    return db_.getUserById(user_id);
}
