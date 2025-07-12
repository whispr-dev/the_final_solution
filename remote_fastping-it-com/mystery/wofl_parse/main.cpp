#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include "avx2_minifier_core.hpp"

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Init Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(9876);           // Replace with your port
    dest.sin_addr.s_addr = inet_addr("161.35.248.233"); // Replace with your IP

    std::string rawJson = R"({"status":"alive","id":"beacon-001","time":1234567890})";
    std::string minified = minify_json_avx2(rawJson);

    std::cout << "Sending minified payload: " << minified << "\n";

    while (true) {
        sendto(sock, minified.c_str(), minified.size(), 0, (sockaddr*)&dest, sizeof(dest));
        std::cout << "Beacon sent.\n";
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
