// listener.cpp
#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9876); // same port as beacon target
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char buffer[512];
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);

    while (true) {
        int recvLen = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&clientAddr, &addrLen);
        if (recvLen > 0) {
            buffer[recvLen] = '\0';
            std::cout << "Received: " << buffer << "\n";
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
