#include "client_connection.h"
#include <iostream>

using json = nlohmann::json;

ClientConnection::ClientConnection(boost::asio::io_context& io_context)
    : io_context_(io_context), resolver_(io_context), socket_(io_context),
      connected_(false), receiving_(false) {}

ClientConnection::~ClientConnection() {
    disconnect();
}

bool ClientConnection::connect(const std::string& host, const std::string& port) {
    try {
        std::cout << "Connecting to " << host << ":" << port << std::endl;
        
        tcp::resolver::results_type endpoints = resolver_.resolve(host, port);
        boost::system::error_code ec;
        
        // Synchronous connect for simplicity
        boost::asio::connect(socket_, endpoints, ec);
        
        if (!ec) {
            connected_ = true;
            std::cout << "Connected successfully!" << std::endl;
            return true;
        } else {
            std::cerr << "Connection failed: " << ec.message() << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Connection error: " << e.what() << std::endl;
        return false;
    }
}

void ClientConnection::disconnect() {
    if (connected_) {
        std::cout << "Disconnecting..." << std::endl;
        connected_ = false;
        receiving_ = false;
        
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
}

void ClientConnection::send(const nlohmann::json& message) {
    try {
        std::string serialized = message.dump() + "\n";
        send(serialized);
    } catch (const std::exception& e) {
        handle_error("Error serializing message: " + std::string(e.what()));
    }
}

void ClientConnection::send(const std::string& raw_message) {
    if (!connected_) {
        handle_error("Not connected to server");
        return;
    }

    try {
        boost::system::error_code ec;
        boost::asio::write(socket_, boost::asio::buffer(raw_message), ec);
        
        if (ec) {
            handle_error("Send failed: " + ec.message());
        } else {
            std::cout << "Sent: " << raw_message.substr(0, raw_message.length()-1) << std::endl; // Remove newline for display
        }
    } catch (const std::exception& e) {
        handle_error("Send error: " + std::string(e.what()));
    }
}

void ClientConnection::startReceiving() {
    std::cout << "startReceiving called - connected_: " << connected_ << ", receiving_: " << receiving_ << std::endl;
    
    if (!connected_ || receiving_) {
        std::cout << "Exiting startReceiving early" << std::endl;
        return;
    }
    
    receiving_ = true;
    std::cout << "Started receiving messages..." << std::endl;
    do_read();
}

void ClientConnection::stopReceiving() {
    receiving_ = false;
    std::cout << "Stopped receiving messages." << std::endl;
}

void ClientConnection::do_read() {
    if (!receiving_ || !connected_) {
        std::cout << "Not reading - receiving_: " << receiving_ << ", connected_: " << connected_ << std::endl;
        return;
    }

    std::cout << "Starting async_read_some..." << std::endl;
    socket_.async_read_some(boost::asio::buffer(read_buffer_),
        [this](boost::system::error_code ec, std::size_t length) {
            std::cout << "Read callback - ec: " << ec.message() << ", length: " << length << std::endl;
            
            if (!ec && receiving_) {
                // Append received data to message buffer
                message_buffer_.append(read_buffer_.data(), length);
                std::cout << "Current buffer: '" << message_buffer_ << "'" << std::endl;
                
                // Process complete messages (separated by newline)
                size_t pos = 0;
                while ((pos = message_buffer_.find('\n')) != std::string::npos) {
                    std::string complete_message = message_buffer_.substr(0, pos);
                    message_buffer_.erase(0, pos + 1);
                    
                    std::cout << "Processing complete message: '" << complete_message << "'" << std::endl;
                    
                    if (!complete_message.empty()) {
                        handle_message(complete_message);
                    }
                }
                
                // Continue reading
                do_read();
            } else if (ec) {
                if (receiving_) { // Only report error if we're still supposed to be receiving
                    handle_error("Read failed: " + ec.message());
                }
                connected_ = false;
                receiving_ = false;
            }
        });
}

void ClientConnection::handle_message(const std::string& message) {
    try {
        std::cout << "Received: " << message << std::endl;
        
        json parsed_message = json::parse(message);
        
        if (message_callback_) {
            message_callback_(parsed_message);
        }
    } catch (const json::parse_error& e) {
        handle_error("JSON parse error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        handle_error("Message handling error: " + std::string(e.what()));
    }
}

void ClientConnection::handle_error(const std::string& error) {
    std::cerr << "ClientConnection Error: " << error << std::endl;
    
    if (error_callback_) {
        error_callback_(error);
    }
}
