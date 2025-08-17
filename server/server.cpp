// Сервер реализует архитектуру Model-Controller в асинхронном виде
// Server -> Session -> JsonParser -> UserManager/Router -> Database

#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <ctime>
#include <nlohmann/json.hpp>
#include "include/common.hpp"
#include "include/database.h"
#include "include/user_manager.h"
#include "include/router.h"
#include "include/json_parser.h"
#include "include/session.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          db_("messenger.db"),
          user_manager_(db_),
          router_(db_, user_manager_),
          json_parser_(std::make_shared<JsonParser>(user_manager_, router_)) {
        
        std::cout << "Server components initialized successfully" << std::endl;
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "New client connected from: " << socket.remote_endpoint() << std::endl;
                    
                    // Создаем новую сессию с общим JsonParser
                    std::make_shared<Session>(std::move(socket), json_parser_)->start();
                } else {
                    std::cerr << "Accept error: " << ec.message() << std::endl;
                }
                
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    Database db_;
    UserManager user_manager_;
    Router router_;
    std::shared_ptr<JsonParser> json_parser_;
};

// static void ShowJsonExamples()
// {
//         // Пример протокола сообщений
//         json example_register;
//         example_register["type"] = "register";
//         example_register["username"] = "user1";
//         example_register["email"] = "user1@example.com";
//         example_register["password"] = "password123";
        
//         json example_login;
//         example_login["type"] = "login";
//         example_login["username"] = "user1";
//         example_login["password"] = "password123";
        
//         json example_message;
//         example_message["type"] = "message";
//         example_message["to"] = "user2";
//         example_message["content"] = "Hello, World!";
        
//         json example_typing;
//         example_typing["type"] = "typing";
//         example_typing["to"] = "user2";
//         example_typing["is_typing"] = true;
        
//         std::cout << "\n=== Message Protocol Examples ===" << std::endl;
//         std::cout << "Register: " << example_register.dump(2) << std::endl;
//         std::cout << "Login: " << example_login.dump(2) << std::endl;
//         std::cout << "Message: " << example_message.dump(2) << std::endl;
//         std::cout << "Typing: " << example_typing.dump(2) << std::endl;
//         std::cout << "================================\n" << std::endl;
// }

int main(int argc, char* argv[]) {
    try {
        int port = PORT;
        if (argc > 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        else if (argc == 2) {
            port = std::atoi(argv[1]);
        }
        std::cout << "Starting Asynchronous Messenger Server on port " << port << std::endl;
        
        boost::asio::io_context io_context;
        Server server(io_context, port);
        std::cout << "Server listening on port " << port << std::endl;

        // ShowJsonExamples();

        io_context.run();
        
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}