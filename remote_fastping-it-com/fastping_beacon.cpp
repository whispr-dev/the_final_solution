// fastping_beacon.cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

// --- Config ---
#define TARGET_IP   "192.168.1.100"  // Change this to your target IP
#define TARGET_PORT 9999
#define INTERVAL_MS 1000             // Beacon interval in milliseconds
// ---------------

std::atomic<bool> running(true);

// Cleanup and exit handler
void signalHandler(int signum) {
    std::cout << "\n[INFO] Caught signal " << signum << ", exiting cleanly.\n";
    running = false;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "[ERROR] WSAStartup failed.\n";
        return 1;
    }
#endif

    // Register signal handlers
    std::signal(SIGINT, signalHandler);   // Ctrl+C
    std::signal(SIGTERM, signalHandler);  // Termination

    // Create socket
    int sock;
#ifdef _WIN32
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[ERROR] Failed to create socket.\n";
        WSACleanup();
        return 1;
    }
#else
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "[ERROR] Failed to create socket.\n";
        return 1;
    }
#endif

    // Target setup
    sockaddr_in dest;
    std::memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(TARGET_PORT);
    dest.sin_addr.s_addr = inet_addr(TARGET_IP);

    std::cout << "[INFO] Sending UDP beacons to " << TARGET_IP << ":" << TARGET_PORT << " every " << INTERVAL_MS << "ms.\n";

    // Beacon loop
    while (running) {
        const char* msg = "fastping.it.com BEACON";
        int sent = sendto(sock, msg, strlen(msg), 0, (sockaddr*)&dest, sizeof(dest));

        if (sent < 0) {
            std::cerr << "[ERROR] Failed to send beacon.\n";
        } else {
            std::cout << "[INFO] Beacon sent.\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_MS));
    }

    // Cleanup
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    std::cout << "[INFO] Exiting fastping beacon.\n";
    return 0;
}
