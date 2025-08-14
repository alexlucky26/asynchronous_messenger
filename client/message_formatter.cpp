#include "message_formatter.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::string MessageFormatter::formatTime(std::time_t timestamp) {
    std::tm* timeinfo = std::localtime(&timestamp);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%H:%M");
    return oss.str();
}

std::string MessageFormatter::getCurrentTimeString() {
    std::time_t now = std::time(nullptr);
    return formatTime(now);
}

std::string MessageFormatter::formatChatMessage(const std::string& sender, const std::string& content, std::time_t timestamp) {
    std::string time_str = formatTime(timestamp);
    std::string formatted_sender = formatUsername(sender);
    return "[" + time_str + "] " + formatted_sender + ": " + content;
}

std::string MessageFormatter::formatSystemMessage(const std::string& message) {
    return "*** " + message + " ***";
}

std::string MessageFormatter::formatTypingStatus(const std::string& username, bool is_typing) {
    if (is_typing) {
        return username + " печатает...";
    } else {
        return ""; // Clear typing status
    }
}

std::string MessageFormatter::formatUsername(const std::string& username, bool is_self) {
    if (is_self) {
        return "Вы";
    }
    return username;
}

std::string MessageFormatter::centerText(const std::string& text, int width) {
    if (text.length() >= width) {
        return text;
    }
    
    int padding = (width - text.length()) / 2;
    return std::string(padding, ' ') + text + std::string(width - text.length() - padding, ' ');
}

std::string MessageFormatter::createSeparator(char character, int width) {
    return std::string(width, character);
}

std::string MessageFormatter::formatPrompt(const std::string& prompt) {
    return prompt + " > ";
}

std::string MessageFormatter::formatError(const std::string& error) {
    return "ОШИБКА: " + error;
}

std::string MessageFormatter::formatSuccess(const std::string& message) {
    return message;
}

std::string MessageFormatter::padLeft(const std::string& str, int width) {
    if (str.length() >= width) {
        return str;
    }
    return std::string(width - str.length(), ' ') + str;
}

std::string MessageFormatter::padRight(const std::string& str, int width) {
    if (str.length() >= width) {
        return str;
    }
    return str + std::string(width - str.length(), ' ');
}
