#include "include/database.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

void Database::initialize() {
    // Create users table
    const char* users_sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "email TEXT NOT NULL UNIQUE,"
        "password_hash TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";

    // Create messages table
    const char* messages_sql = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sender_id INTEGER NOT NULL,"
        "receiver_id INTEGER NOT NULL,"
        "content TEXT NOT NULL,"
        "sent_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "is_file BOOLEAN DEFAULT FALSE,"
        "file_path TEXT,"
        "FOREIGN KEY(sender_id) REFERENCES users(id),"
        "FOREIGN KEY(receiver_id) REFERENCES users(id));";

    char* err_msg = nullptr;
    
    // Execute users table creation
    if (sqlite3_exec(db_, users_sql, 0, 0, &err_msg) != SQLITE_OK) {
        std::string error = "SQL error on users table creation: ";
        error += err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error);
    }
    
    // Execute messages table creation
    if (sqlite3_exec(db_, messages_sql, 0, 0, &err_msg) != SQLITE_OK) {
        std::string error = "SQL error on messages table creation: ";
        error += err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error);
    }
    
    std::cout << "Database tables initialized successfully." << std::endl;
}

bool Database::addUser(const std::string& username, const std::string& email, const std::string& password_hash) {
    const char* sql = "INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, password_hash.c_str(), -1, SQLITE_STATIC);
    
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (result == SQLITE_DONE) {
        std::cout << "User " << username << " registered successfully." << std::endl;
        return true;
    } else {
        std::cerr << "Failed to register user: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
}

std::unique_ptr<User> Database::getUser(const std::string& username) {
    const char* sql = "SELECT id, username, email, password_hash, created_at FROM users WHERE username = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    }
    
    sqlite3_finalize(stmt);
    return user;
}

std::unique_ptr<User> Database::getUserById(int user_id) {
    const char* sql = "SELECT id, username, email, password_hash, created_at FROM users WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return nullptr;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    }
    
    sqlite3_finalize(stmt);
    return user;
}

bool Database::storeMessage(int sender_id, int receiver_id, const std::string& content, bool is_file, const std::string& file_path) {
    const char* sql = "INSERT INTO messages (sender_id, receiver_id, content, is_file, file_path) VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, sender_id);
    sqlite3_bind_int(stmt, 2, receiver_id);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, is_file ? 1 : 0);
    sqlite3_bind_text(stmt, 5, file_path.c_str(), -1, SQLITE_STATIC);
    
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

std::vector<Message> Database::getMessages(int user_id, int other_user_id, int limit) {
    const char* sql = "SELECT id, sender_id, receiver_id, content, sent_at, is_file, file_path FROM messages "
                      "WHERE (sender_id = ? AND receiver_id = ?) OR (sender_id = ? AND receiver_id = ?) "
                      "ORDER BY sent_at DESC LIMIT ?;";
    
    sqlite3_stmt* stmt;
    std::vector<Message> messages;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return messages;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, other_user_id);
    sqlite3_bind_int(stmt, 3, other_user_id);
    sqlite3_bind_int(stmt, 4, user_id);
    sqlite3_bind_int(stmt, 5, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message msg;
        msg.id = sqlite3_column_int(stmt, 0);
        msg.sender_id = sqlite3_column_int(stmt, 1);
        msg.receiver_id = sqlite3_column_int(stmt, 2);
        msg.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        msg.sent_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        msg.is_file = sqlite3_column_int(stmt, 5) != 0;
        
        const char* file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        msg.file_path = file_path ? file_path : "";
        
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

std::vector<Message> Database::getOfflineMessages(int user_id) {
    const char* sql = "SELECT id, sender_id, receiver_id, content, sent_at, is_file, file_path FROM messages "
                      "WHERE receiver_id = ? ORDER BY sent_at ASC;";
    
    sqlite3_stmt* stmt;
    std::vector<Message> messages;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return messages;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message msg;
        msg.id = sqlite3_column_int(stmt, 0);
        msg.sender_id = sqlite3_column_int(stmt, 1);
        msg.receiver_id = sqlite3_column_int(stmt, 2);
        msg.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        msg.sent_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        msg.is_file = sqlite3_column_int(stmt, 5) != 0;
        
        const char* file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        msg.file_path = file_path ? file_path : "";
        
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}
