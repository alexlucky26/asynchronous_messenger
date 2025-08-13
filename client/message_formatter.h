#ifndef MESSAGE_FORMATTER_H
#define MESSAGE_FORMATTER_H

#include <string>
#include <ctime>

class MessageFormatter {
public:
    MessageFormatter() = default;
    ~MessageFormatter() = default;

    // Time formatting
    static std::string formatTime(std::time_t timestamp);
    static std::string getCurrentTimeString();
    
    // Message formatting
    static std::string formatChatMessage(const std::string& sender, const std::string& content, std::time_t timestamp);
    static std::string formatSystemMessage(const std::string& message);
    static std::string formatTypingStatus(const std::string& username, bool is_typing);
    
    // Text formatting helpers
    static std::string formatUsername(const std::string& username, bool is_self = false);
    static std::string centerText(const std::string& text, int width = 80);
    static std::string createSeparator(char character = '=', int width = 80);
    
    // Input formatting
    static std::string formatPrompt(const std::string& prompt);
    static std::string formatError(const std::string& error);
    static std::string formatSuccess(const std::string& message);

private:
    static std::string padLeft(const std::string& str, int width);
    static std::string padRight(const std::string& str, int width);
};

#endif // MESSAGE_FORMATTER_H
