#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <array>

using boost::asio::ip::tcp;

class JsonParser; // Forward declaration

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, std::shared_ptr<JsonParser> json_parser);
    ~Session();

    void start();
    void send(const nlohmann::json& message);
    
    // Authentication state
    void setAuthenticated(int user_id, const std::string& username);
    bool isAuthenticated() const { return authenticated_; }
    int getUserId() const { return user_id_; }
    const std::string& getUsername() const { return username_; }
    
    // Session management
    void close();

private:
    void do_read();
    void do_write();
    void handle_message(const std::string& message);
    
    tcp::socket socket_;
    std::shared_ptr<JsonParser> json_parser_;
    
    // Authentication state
    bool authenticated_;
    int user_id_;
    std::string username_;
    
    // Buffer management
    std::array<char, 8192> read_buffer_;
    std::string write_buffer_;
    std::string message_buffer_; // For accumulating partial messages
    
    // Session state
    bool closing_;
};

#endif // SESSION_HPP
