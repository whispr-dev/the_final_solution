// fastping_icmp.cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define TARGET_IP "8.8.8.8"
#define TIMEOUT_MS 1000

std::atomic<bool> running(true);

void signalHandler(int signum) {
    std::cout << "\n[INFO] Signal " << signum << " received. Exiting.\n";
    running = false;
}

// ICMP Header Structure
struct ICMPHeader {
    BYTE type;
    BYTE code;
    USHORT checksum;
    USHORT id;
    USHORT seq;
};

USHORT checksum(USHORT* buffer, int size) {
    unsigned long sum = 0;
    while (size > 1) {
        sum += *buffer++;
        size -= 2;
    }
    if (size) sum += *(UCHAR*)buffer;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (USHORT)(~sum);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "[ERROR] WSAStartup failed.\n";
        return 1;
    }

    std::signal(SIGINT, signalHandler);

    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[ERROR] Failed to create raw socket (Admin rights required).\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in target;
    target.sin_family = AF_INET;
    inet_pton(AF_INET, TARGET_IP, &target.sin_addr);

    USHORT seq = 0;
    ICMPHeader icmp{ 8, 0, 0, (USHORT)GetCurrentProcessId(), seq };

    std::cout << "[INFO] ICMP Ping to " << TARGET_IP << "\n";

    while (running) {
        char packet[sizeof(ICMPHeader)];
        icmp.seq = seq++;
        icmp.checksum = 0;
        memcpy(packet, &icmp, sizeof(ICMPHeader));
        icmp.checksum = checksum((USHORT*)packet, sizeof(ICMPHeader));
        memcpy(packet, &icmp, sizeof(ICMPHeader));

        auto start = std::chrono::high_resolution_clock::now();

        sendto(sock, packet, sizeof(packet), 0, (sockaddr*)&target, sizeof(target));

        char recvBuf[1024];
        sockaddr_in from;
        int fromLen = sizeof(from);

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);
        timeval timeout = { TIMEOUT_MS / 1000, (TIMEOUT_MS % 1000) * 1000 };

        if (select(0, &readSet, nullptr, nullptr, &timeout) > 0) {
            int received = recvfrom(sock, recvBuf, sizeof(recvBuf), 0, (sockaddr*)&from, &fromLen);
            if (received > 0) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "[REPLY] From " << inet_ntoa(from.sin_addr) << " - " << duration.count() << "ms\n";
            }
        } else {
            std::cout << "[TIMEOUT] No reply.\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
