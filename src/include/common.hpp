#ifndef COMMON_HPP
#define COMMON_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <string>

const int PORT = 12345;
const std::string SERVER_ADDRESS = "127.0.0.1";

void log(const std::string& message) {
    std::cout << message << std::endl;
}

#endif // COMMON_HPP