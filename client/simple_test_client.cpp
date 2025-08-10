#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>

using boost::asio::ip::tcp;
using json = nlohmann::json;

int main() {
    try {
        boost::asio::io_context io_context;
        
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "9999");
        
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        
        std::cout << "Connected to server!" << std::endl;
        
        // Test 1: Register user
        json register_msg;
        register_msg["type"] = "register";
        register_msg["username"] = "testuser1";
        register_msg["email"] = "test1@example.com";
        register_msg["password"] = "testpassword";
        
        std::string serialized = register_msg.dump() + "\n";
        boost::asio::write(socket, boost::asio::buffer(serialized));
        std::cout << "Sent registration request" << std::endl;
        
        // Read response
        boost::asio::streambuf response_buffer;
        boost::asio::read_until(socket, response_buffer, "\n");
        
        std::istream response_stream(&response_buffer);
        std::string response_line;
        std::getline(response_stream, response_line);
        
        json response = json::parse(response_line);
        std::cout << "Registration response: " << response.dump(2) << std::endl;
        
        // Test 2: Login
        json login_msg;
        login_msg["type"] = "login";
        login_msg["username"] = "testuser1";
        login_msg["password"] = "testpassword";
        
        serialized = login_msg.dump() + "\n";
        boost::asio::write(socket, boost::asio::buffer(serialized));
        std::cout << "Sent login request" << std::endl;
        
        // Read login response
        boost::asio::read_until(socket, response_buffer, "\n");
        response_stream.clear();
        std::getline(response_stream, response_line);
        
        response = json::parse(response_line);
        std::cout << "Login response: " << response.dump(2) << std::endl;
        
        // Test 3: Send message
        json message_msg;
        message_msg["type"] = "message";
        message_msg["to"] = "testuser2";
        message_msg["content"] = "Hello from simple test client!";
        
        serialized = message_msg.dump() + "\n";
        boost::asio::write(socket, boost::asio::buffer(serialized));
        std::cout << "Sent message request" << std::endl;
        
        // Read message response
        boost::asio::read_until(socket, response_buffer, "\n");
        response_stream.clear();
        std::getline(response_stream, response_line);
        
        response = json::parse(response_line);
        std::cout << "Message response: " << response.dump(2) << std::endl;
        
        socket.close();
        std::cout << "Test completed successfully!" << std::endl;
        
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
