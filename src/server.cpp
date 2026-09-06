#include "server.h"
#include "command.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#ifdef DELETE
#undef DELETE
#endif

Server::Server(int port, KVStore& store)
    : port_(port), store_(store) {
}

static std::string recvLine(SOCKET clientSocket) {
    std::string line;
    char ch;
    while (true) {
        int result = recv(clientSocket, &ch, 1, 0);
        if (result <= 0) {
            return "";
        }
        if (ch == '\n') {
            break;
        }
        if (ch != '\r') {
            line += ch;
        }
    }
    return line;
}

static void sendLine(SOCKET clientSocket, const std::string& line) {
    std::string toSend = line + "\n";
    send(clientSocket, toSend.c_str(), static_cast<int>(toSend.size()), 0);
}

// Runs on its own thread — handles one client's commands until they disconnect.
static void handleClient(SOCKET clientSocket, KVStore& store) {
    std::cout << "Client connected! (thread "
              << std::this_thread::get_id() << ")" << std::endl;

    while (true) {
        std::string line = recvLine(clientSocket);
        if (line.empty()) {
            break;
        }

        Command cmd = parseCommand(line);

        switch (cmd.type) {
            case CommandType::SET:
                store.put(cmd.key, cmd.value);
                sendLine(clientSocket, "OK");
                break;
            case CommandType::GET: {
                auto val = store.get(cmd.key);
                sendLine(clientSocket, val ? *val : "NOT_FOUND");
                break;
            }
            case CommandType::DELETE:
                store.remove(cmd.key);
                sendLine(clientSocket, "OK");
                break;
            case CommandType::INVALID:
                sendLine(clientSocket, "ERROR: missing arguments");
                break;
            case CommandType::UNKNOWN:
                sendLine(clientSocket, "ERROR: unknown command");
                break;
        }
    }

    std::cout << "Client disconnected. (thread "
              << std::this_thread::get_id() << ")" << std::endl;
    closesocket(clientSocket);
}

void Server::run() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("socket() failed");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port_));

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("bind() failed");
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("listen() failed");
    }

    std::cout << "Server listening on port " << port_ << "..." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept() failed, continuing..." << std::endl;
            continue;
        }

        // Spawn a dedicated thread for this client, then immediately loop
        // back to accept() so the next client isn't blocked.
        std::thread clientThread(handleClient, clientSocket, std::ref(store_));
        clientThread.detach();
    }

    closesocket(listenSocket);
    WSACleanup();
}