#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include "include/common.hpp"

using boost::asio::ip::tcp;

class Client {
public:
    Client(boost::asio::io_context& io_context, const std::string& host, const std::string& port)
        : resolver_(io_context), socket_(io_context) {
        connect(host, port);
    }

private:
    void connect(const std::string& host, const std::string& port) {
        tcp::resolver::results_type endpoints = resolver_.resolve(host, port);
        boost::asio::async_connect(socket_, endpoints,
            [this](boost::system::error_code ec, tcp::endpoint) {
                if (!ec) {
                    send_request();
                }
            });
    }

    void send_request() {
        std::string request = "Hello from client!";
        boost::asio::async_write(socket_, boost::asio::buffer(request),
            [this](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    receive_response();
                }
            });
    }

    void receive_response() {
        boost::asio::async_read_until(socket_, response_buffer_, "\n",
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::istream response_stream(&response_buffer_);
                    std::string response;
                    std::getline(response_stream, response);
                    std::cout << "Response from server: " << response << std::endl;
                }
            });
    }

    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::asio::streambuf response_buffer_;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: client <host> <port>\n";
        return 1;
    }

    boost::asio::io_context io_context;
    Client client(io_context, argv[1], argv[2]);
    io_context.run();

    return 0;
}