// Сервер должен реализовывать:
// Обработку подключений клиентов с использованием Boost.Asio (асинхронно)
// Регистрацию и авторизацию пользователей (с проверкой и хешированием пароля)
// Хранение пользовательских данных и истории сообщений в БД (SQLite)
// Асинхронную маршрутизацию сообщений между пользователями
// Сохранение сообщений для оффлайн-пользователей
// Уведомления о статусе "typing"
// (Опционально) Уничтожение сессии клиента при неактивности по таймауту
// (Опционально) Приём и передача файлов через сервер

#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <ctime>
#include <nlohmann/json.hpp>
#include "include/common.hpp"
#include "include/database.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    do_write(length);
                }
            });
    }

    void do_write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    do_read();
                }
            });
    }

    tcp::socket socket_;
    char data_[1024];
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          db_("messenger.db") {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "New client connected!" << std::endl;
                    std::make_shared<Session>(std::move(socket))->start();
                } else {
                    std::cerr << "Accept error: " << ec.message() << std::endl;
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    Database db_;
};

int main(int argc, char* argv[]) {
    try  {
        int port = PORT;
        if (argc > 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        else if (argc == 2)
        {
            int port = std::atoi(argv[1]);
        }

        
        std::cout << "Starting server on port " << port << std::endl;
        
        boost::asio::io_context io_context;
        Server server(io_context, port);
        
        std::cout << "Server listening on port " << port << std::endl;

        // Пример JSON сообщения
        json example_message;
        example_message["type"] = "message";
        example_message["from"] = "user1";
        example_message["to"] = "user2";
        example_message["content"] = "Hello, World!";
        example_message["timestamp"] = std::time(nullptr);
        
        std::cout << "Example JSON message: " << example_message.dump(4) << std::endl;
        
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}