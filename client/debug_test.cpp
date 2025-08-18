// Simple test to debug client-server communication
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include "include/client_connection.h"

using json = nlohmann::json;

int main() {
    try {
        std::cout << "=== CLIENT-SERVER COMMUNICATION TEST ===" << std::endl;
        
        boost::asio::io_context io_context;
        
        auto connection = std::make_shared<ClientConnection>(io_context);
        
        // Set message callback
        connection->setMessageCallback([](const json& message) {
            std::cout << "✅ RECEIVED FROM SERVER: " << message.dump(2) << std::endl;
        });
        
        // Start IO context in separate thread
        std::thread io_thread([&io_context]() {
            std::cout << "IO thread started" << std::endl;
            try {
                // Keep io_context alive with work guard
                boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(io_context.get_executor());
                io_context.run();
                std::cout << "IO thread finished normally" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "IO thread error: " << e.what() << std::endl;
            }
        });
        
        // Connect
        if (!connection->connect("127.0.0.1", "9999")) {
            std::cerr << "Failed to connect!" << std::endl;
            return 1;
        }
        
        // Start receiving
        connection->startReceiving();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Send login request
        json login_request;
        login_request["type"] = "login";
        login_request["username"] = "alex";
        login_request["password"] = "1234";
        
        std::cout << "📤 SENDING TO SERVER: " << login_request.dump() << std::endl;
        connection->send(login_request);
        
        // Wait for response - give more time for IO thread to process
        std::cout << "Waiting for server response..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Check if we received anything
        std::cout << "Waiting complete. Shutting down in 1 second..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        std::cout << "Test completed, shutting down..." << std::endl;
        connection->disconnect();
        io_context.stop();
        
        if (io_thread.joinable()) {
            io_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
