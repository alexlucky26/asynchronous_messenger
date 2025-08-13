# Client CMakeLists.txt

# Conan устанавливает CMAKE_INCLUDE_PATH через toolchain, но иногда нужно добавить явно
foreach(path IN LISTS CMAKE_INCLUDE_PATH)
    if(EXISTS ${path})
        include_directories("${path}")
        message(STATUS "Client: Adding include path: ${path}")
    endif()
endforeach()

# Добавляем library paths тоже
foreach(path IN LISTS CMAKE_LIBRARY_PATH)
    if(EXISTS ${path})
        link_directories("${path}")
        message(STATUS "Client: Adding library path: ${path}")
    endif()
endforeach()

# Create client executables
add_executable(client 
    client.cpp
    client_connection.cpp
    client_state_machine.cpp
    message_receiver.cpp
    message_formatter.cpp
    client_ui.cpp
    local_history_store.cpp
)

add_executable(simple_test_client simple_test_client.cpp)
add_executable(debug_test 
    debug_test.cpp
    client_connection.cpp
)

# Set C++ properties and include directories
target_include_directories(client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${Boost_INCLUDE_DIRS}
)

target_include_directories(simple_test_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${Boost_INCLUDE_DIRS}
)

target_include_directories(debug_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${Boost_INCLUDE_DIRS}
)

# Platform-specific libraries for clients
if(WIN32)
    target_link_libraries(client 
        PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
        ws2_32
        mswsock  
    )
    target_link_libraries(simple_test_client 
        PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
        ws2_32
        mswsock  
    )
    target_link_libraries(debug_test 
        PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
        ws2_32
        mswsock  
    )
    
elseif(UNIX)
    target_link_libraries(client 
        PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
        pthread
    )
    target_link_libraries(simple_test_client 
        PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
        pthread
    )
    
elseif(APPLE)
    target_link_libraries(client 
        PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
        pthread
    )
    target_link_libraries(simple_test_client 
        PRIVATE
        Boost::system
        nlohmann_json::nlohmann_json
        pthread
    )
endif()

# Set output directory for executables
set_target_properties(client simple_test_client debug_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
