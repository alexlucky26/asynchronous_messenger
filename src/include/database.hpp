#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>

struct User {
    int id;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string created_at;
};

struct Message {
    int id;
    int sender_id;
    int receiver_id;
    std::string content;
    std::string sent_at;
    bool is_file;
    std::string file_path;
};

class Database {
public:
    Database(const std::string& db_path) {
        if (sqlite3_open(db_path.c_str(), &db_)) {
            std::string error_msg = "Can't open database: ";
            error_msg += sqlite3_errmsg(db_);
            throw std::runtime_error(error_msg);
        } else {
            std::cout << "Opened database successfully" << std::endl;
        }
        initialize();
    }

    ~Database() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    // User operations
    bool addUser(const std::string& username, const std::string& email, const std::string& password_hash);
    std::unique_ptr<User> getUser(const std::string& username);
    std::unique_ptr<User> getUserById(int user_id);
    
    // Message operations
    bool storeMessage(int sender_id, int receiver_id, const std::string& content, bool is_file = false, const std::string& file_path = "");
    std::vector<Message> getMessages(int user_id, int other_user_id, int limit = 50);
    std::vector<Message> getOfflineMessages(int user_id);

private:
    void initialize();
    sqlite3* db_ = nullptr;
};

#endif // DATABASE_HPP