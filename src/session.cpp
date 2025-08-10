#include "include/session.hpp"
#include "include/json_parser.hpp"
#include "include/user_manager.hpp"
#include <iostream>

Session::Session(tcp::socket socket, std::shared_ptr<JsonParser> json_parser)
    : socket_(std::move(socket)), json_parser_(json_parser), 
      authenticated_(false), user_id_(-1), closing_(false) {}

Session::~Session() {
    std::cout << "Session destroyed for user: " << username_ << std::endl;
}

void Session::start() {
    std::cout << "New session started" << std::endl;
    do_read();
}

void Session::send(const nlohmann::json& message) {
    try {
        std::string serialized = message.dump() + "\n";  // Добавляем разделитель
        
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(serialized),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                    close();
                }
            });
    } catch (const std::exception& e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
    }
}

void Session::setAuthenticated(int user_id, const std::string& username) {
    authenticated_ = true;
    user_id_ = user_id;
    username_ = username;
    std::cout << "Session authenticated for user: " << username << " (ID: " << user_id << ")" << std::endl;
}

void Session::close() {
    if (!closing_) {
        closing_ = true;
        
        std::cout << "Closing session for user: " << username_ << std::endl;
        
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
}

void Session::do_read() {
    auto self(shared_from_this());
    
    socket_.async_read_some(boost::asio::buffer(read_buffer_),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                // Добавляем полученные данные к буферу сообщений
                message_buffer_.append(read_buffer_.data(), length);
                
                // Обрабатываем все полные сообщения в буфере
                size_t pos = 0;
                while ((pos = message_buffer_.find('\n')) != std::string::npos) {
                    std::string complete_message = message_buffer_.substr(0, pos);
                    message_buffer_.erase(0, pos + 1);
                    
                    if (!complete_message.empty()) {
                        handle_message(complete_message);
                    }
                }
                
                do_read();
            } else {
                std::cout << "Read error: " << ec.message() << std::endl;
                close();
            }
        });
}

void Session::handle_message(const std::string& message) {
    try {
        std::cout << "Handling message: " << message << std::endl;
        json_parser_->parseMessage(message, shared_from_this());
    } catch (const std::exception& e) {
        std::cerr << "Error handling message: " << e.what() << std::endl;
    }
}
