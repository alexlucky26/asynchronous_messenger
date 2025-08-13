#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <array>
#include <functional>

using boost::asio::ip::tcp;

class ClientConnection {
public:
    using MessageCallback = std::function<void(const nlohmann::json&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    ClientConnection(boost::asio::io_context& io_context);
    ~ClientConnection();

    // Connection management
    bool connect(const std::string& host, const std::string& port);
    void disconnect();
    bool isConnected() const { return connected_; }

    // Message sending (async)
    void send(const nlohmann::json& message);
    void send(const std::string& raw_message);

    // Message receiving (async)
    void startReceiving();
    void stopReceiving();

    // Callbacks
    void setMessageCallback(MessageCallback callback) { message_callback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }

private:
    void do_read();
    void handle_message(const std::string& message);
    void handle_error(const std::string& error);

    boost::asio::io_context& io_context_;
    tcp::resolver resolver_;
    tcp::socket socket_;
    
    // State
    bool connected_;
    bool receiving_;

    // Buffers
    std::array<char, 8192> read_buffer_;
    std::string message_buffer_; // For accumulating partial messages
    
    // Callbacks
    MessageCallback message_callback_;
    ErrorCallback error_callback_;
};

#endif // CLIENT_CONNECTION_H
