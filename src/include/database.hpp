#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <stdexcept>

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

private:
    void initialize() {
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT NOT NULL UNIQUE,"
            "password_hash TEXT NOT NULL);";

        char* err_msg = nullptr;
        if (sqlite3_exec(db_, sql, 0, 0, &err_msg) != SQLITE_OK) {
            std::string error = "SQL error on table creation: ";
            error += err_msg;
            sqlite3_free(err_msg);
            throw std::runtime_error(error);
        }
        std::cout << "Users table initialized successfully." << std::endl;
    }

    sqlite3* db_ = nullptr;
};

#endif // DATABASE_HPP