#include "server.h"
#include <iostream>
#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

Server::Server(int port, KVStore& store)
    : port_(port), store_(store) {
}

void Server::run() {
    // 1. Initialize Winsock (Windows-specific requirement)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    // 2. Create a socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("socket() failed");
    }

    // 3. Bind it to the given port, on all local interfaces
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port_));

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("bind() failed");
    }

    // 4. Start listening for connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("listen() failed");
    }

    std::cout << "Server listening on port " << port_ << "..." << std::endl;

    // 5. Accept ONE client connection (proof of concept — loop comes later)
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("accept() failed");
    }

    std::cout << "Client connected!" << std::endl;

    // TEMPORARY: just close the connection immediately, proving accept() worked
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
}