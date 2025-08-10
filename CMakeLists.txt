cmake_minimum_required(VERSION 3.15)
project(messenger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define Windows version for Boost.Asio
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7
endif()

# Add Config directories to CMAKE_PREFIX_PATH and CMAKE_MODULE_PATH
list(APPEND CMAKE_PREFIX_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/Config/cmake"
)

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/Config/cmake"
)

# Include Conan toolchain if it exists in Config directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Config/cmake/conan_toolchain.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/Config/cmake/conan_toolchain.cmake")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conan_toolchain.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/conan_toolchain.cmake")
endif()

# Include Conan dependencies
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Config/cmake/conandeps_legacy.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/Config/cmake/conandeps_legacy.cmake")
endif()

# Find Boost package with required components
find_package(Boost REQUIRED COMPONENTS system)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Add src subdirectory
add_subdirectory(src)