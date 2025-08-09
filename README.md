# C++ Asynchronous Server and Client Application

This project is a C++ application that demonstrates the use of Boost.Asio for creating an asynchronous server and client. The server listens for incoming connections and handles client requests asynchronously, while the client connects to the server and sends requests, also handling responses asynchronously.

## Project Structure

```
cpp-async-app
├── src
│   ├── server.cpp        # Implementation of the asynchronous server
│   ├── client.cpp        # Implementation of the asynchronous client
│   └── include
│       └── common.hpp    # Common definitions and declarations
├── CMakeLists.txt        # CMake configuration file
├── conanfile.txt         # Conan configuration file for dependencies
└── README.md             # Project documentation
```

## Requirements

- C++11 or higher
- CMake
- Conan
- Boost.Asio

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/microsoft/vscode-remote-try-cpp.git
cd cpp-async-app
```

### 2. Install Dependencies

Make sure you have Conan installed. If not, you can install it using pip:

```bash
pip install conan
```

Then, install the project dependencies:

```bash
conan install . --build=missing
```

### 3. Build the Project

Use CMake to build the project:

```bash
mkdir build
cd build
cmake ..
make
```

### 4. Run the Server and Client

In one terminal, run the server:

```bash
./server
```

In another terminal, run the client:

```bash
./client
```

## License

This project is licensed under the MIT License. See the LICENSE file for more details.