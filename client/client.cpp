// ASYNCHRONOUS MESSENGER CLIENT
// Полнофункциональный клиент с поддержкой:
// Регистрация пользователя
// Авторизация пользователя  
// Отправка текстовых сообщений
// Получение сообщений в реальном времени
// Отображение статуса "пользователь печатает"
// (Опционально) Передача файлов
// (Опционально) Локальное хранение истории сообщений

#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include "include/common.hpp"
#include "include/client_connection.h"
#include "include/client_state_machine.h"
#include "include/message_receiver.h"
#include "include/client_ui.h"

int main(int argc, char* argv[]) {
    try {
        std::locale::global(std::locale("ru_RU.UTF-8"));
        std::wcout.imbue(std::locale());

        std::cout << "Запуск Asynchronous Messenger Client..." << std::endl;
        
        // Create IO context for async operations
        boost::asio::io_context io_context;
        
        // Create connection manager
        auto connection = std::make_shared<ClientConnection>(io_context);
        
        // Create state machine
        auto state_machine = std::make_shared<ClientStateMachine>(connection);
        
        // Create message receiver
        auto message_receiver = std::make_shared<MessageReceiver>(connection, state_machine);
        
        // Set message receiver in state machine
        state_machine->setMessageReceiver(message_receiver);
        
        // Create UI
        auto ui = std::make_unique<ClientUI>(state_machine, message_receiver);
        
        // Run IO context in separate thread
        std::thread io_thread([&io_context]() {
            std::cout << "IO thread started" << std::endl;
            try {
                // Keep io_context alive with work guard to prevent premature exit
                boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(io_context.get_executor());
                io_context.run();
                std::cout << "IO thread finished" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "IO thread error: " << e.what() << std::endl;
            }
        });
        
        // Run UI in main thread
        ui->run();
        
        // Cleanup
        std::cout << "Завершение работы клиента..." << std::endl;
        io_context.stop();
        
        if (io_thread.joinable()) {
            io_thread.join();
        }
        
        std::cout << "Клиент завершил работу." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка клиента: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}