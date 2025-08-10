# Include directories
include_directories(include)

# Conan устанавливает CMAKE_INCLUDE_PATH через toolchain, но иногда нужно добавить явно
# Используем пути из CMAKE_INCLUDE_PATH, которые установил conan_toolchain.cmake
foreach(path IN LISTS CMAKE_INCLUDE_PATH)
    if(EXISTS ${path})
        include_directories("${path}")
        message(STATUS "Adding include path: ${path}")
    endif()
endforeach()

# Добавляем library paths тоже
foreach(path IN LISTS CMAKE_LIBRARY_PATH)
    if(EXISTS ${path})
        link_directories("${path}")
        message(STATUS "Adding library path: ${path}")
    endif()
endforeach()

# Create executables
add_executable(server server.cpp)
add_executable(client client.cpp)

# Set C++ properties and include directories
target_include_directories(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}    
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)
target_include_directories(client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${Boost_INCLUDE_DIRS}
)

# Platform-specific libraries
if(WIN32)
    # Windows CRT libraries for SQLite (if needed)
    if(MSVC)
        target_link_libraries(server 
            PRIVATE
            Boost::system
            sqlite3
            ws2_32      # Windows Sockets 2.0
            mswsock     # Microsoft Winsock
            ucrt
            msvcrt
        )
    else()
        target_link_libraries(server 
            PRIVATE
            Boost::system
            sqlite3
            ws2_32      # Windows Sockets 2.0
            mswsock     # Microsoft Winsock
        )
    endif()

    target_link_libraries(client 
        PRIVATE
        Boost::system
        ws2_32
        mswsock  
    )
    
elseif(UNIX)
    # Linux/Unix libraries
    target_link_libraries(server 
        PRIVATE
        Boost::system
        sqlite3
        pthread     # POSIX threads
        dl          # Dynamic loading
    )
    target_link_libraries(client 
        PRIVATE
        Boost::system
        pthread
    )
    
elseif(APPLE)
    # macOS specific libraries (if needed)
    target_link_libraries(server 
        PRIVATE
        Boost::system
        sqlite3
        pthread
    )
    target_link_libraries(client 
        PRIVATE
        Boost::system
        pthread
    )
endif()

# Set output directory for executables (optional)
set_target_properties(server client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
