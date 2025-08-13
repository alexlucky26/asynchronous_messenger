#ifndef LOCAL_HISTORY_STORE_H
#define LOCAL_HISTORY_STORE_H

#include <string>
#include <vector>
#include <ctime>

// Message structure for local storage
struct LocalMessage {
    std::string sender;
    std::string content;
    std::time_t timestamp;
    bool is_self;
    
    LocalMessage(const std::string& sender, const std::string& content, std::time_t timestamp, bool is_self = false)
        : sender(sender), content(content), timestamp(timestamp), is_self(is_self) {}
};

// Optional class for future implementation of local message history
class LocalHistoryStore {
public:
    LocalHistoryStore(const std::string& username);
    ~LocalHistoryStore() = default;

    // Save messages locally (future implementation)
    void saveMessage(const std::string& sender, const std::string& content, std::time_t timestamp);
    
    // Load message history (future implementation)
    std::vector<LocalMessage> loadHistory(const std::string& chat_partner, int limit = 50);
    
    // Clear history (future implementation)
    void clearHistory(const std::string& chat_partner = "");
    
    // Enable/disable local storage
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

private:
    std::string username_;
    std::string history_file_;
    bool enabled_;
    
    // Future: SQLite or file-based storage implementation
    void initializeStorage();
    void writeToFile(const LocalMessage& message);
};

#endif // LOCAL_HISTORY_STORE_H
