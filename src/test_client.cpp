#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    int port = 6380;
    if (argc >= 2) {
        port = std::atoi(argv[1]);
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<u_short>(port));
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "connect() failed on port " << port << std::endl;
        return 1;
    }

    std::cout << "Connected to server on port " << port << ". Type commands (Ctrl+C to quit):" << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::string toSend = line + "\n";
        send(sock, toSend.c_str(), static_cast<int>(toSend.size()), 0);

        char buffer[1024] = {0};
        int received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            std::cout << "Server closed connection." << std::endl;
            break;
        }
        std::cout << buffer;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}