# Include directories
include_directories(include)

# Add Boost include directory explicitly 
include_directories("C:/Users/Александр/.conan2/p/boost0a77eeb8c45bf/p/include")

# Create executables
add_executable(server server.cpp)
add_executable(client client.cpp)

# Set C++ properties and include directories
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link with Boost libraries
target_link_libraries(server 
    Boost::system
)
target_link_libraries(client 
    Boost::system
)

# For Windows, link with Winsock
if(WIN32)
    target_link_libraries(server ws2_32 mswsock)
    target_link_libraries(client ws2_32 mswsock)
endif()

# Set output directory for executables (optional)
set_target_properties(server client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
