#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

const char* TARGET_IP = "161.35.248.233";
const int TARGET_PORT = 9999;
const int BEACON_INTERVAL_MS = 10000;

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(TARGET_PORT);
    dest.sin_addr.s_addr = inet_addr(TARGET_IP);

    const char* message = "BEACON";

    while (true) {
        sendto(sock, message, strlen(message), 0, (sockaddr*)&dest, sizeof(dest));
        std::cout << "Beacon beaming out to " << TARGET_IP << ":" << TARGET_PORT << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(BEACON_INTERVAL_MS));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
