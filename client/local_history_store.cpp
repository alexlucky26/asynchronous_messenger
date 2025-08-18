#include "include/local_history_store.h"
#include <iostream>

LocalHistoryStore::LocalHistoryStore(const std::string& username)
    : username_(username), enabled_(false) {
    history_file_ = username + "_history.txt";
    initializeStorage();
}

void LocalHistoryStore::saveMessage(const std::string& sender, const std::string& content, std::time_t timestamp) {
    if (!enabled_) {
        return;
    }
    
    // TODO: Implement local message storage
    // For now, just a placeholder
    std::cout << "LocalHistoryStore: Would save message from " << sender << std::endl;
}

std::vector<LocalMessage> LocalHistoryStore::loadHistory(const std::string& chat_partner, int limit) {
    std::vector<LocalMessage> messages;
    
    if (!enabled_) {
        return messages;
    }
    
    // TODO: Implement local message loading
    // For now, return empty vector
    std::cout << "LocalHistoryStore: Would load history with " << chat_partner << std::endl;
    
    return messages;
}

void LocalHistoryStore::clearHistory(const std::string& chat_partner) {
    if (!enabled_) {
        return;
    }
    
    // TODO: Implement history clearing
    std::cout << "LocalHistoryStore: Would clear history" << std::endl;
}

void LocalHistoryStore::initializeStorage() {
    // TODO: Initialize SQLite database or file-based storage
    std::cout << "LocalHistoryStore: Initialized for user " << username_ << std::endl;
}

void LocalHistoryStore::writeToFile(const LocalMessage& message) {
    // TODO: Write message to storage
}
