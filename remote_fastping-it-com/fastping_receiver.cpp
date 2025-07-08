// fastping_receiver.cpp
#include <iostream>
#include <thread>
#include <atomic>
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
#define LISTEN_PORT 9999
#define BUFFER_SIZE 512
// ---------------

std::atomic<bool> running(true);

// Signal handler for graceful exit
void signalHandler(int signum) {
    std::cout << "\n[INFO] Caught signal " << signum << ", shutting down receiver.\n";
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

    // Create UDP socket
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

    // Bind to listen port
    sockaddr_in localAddr;
    std::memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(LISTEN_PORT);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        std::cerr << "[ERROR] Failed to bind socket.\n";
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    std::cout << "[INFO] Listening for beacons on port " << LISTEN_PORT << "...\n";

    // Receive loop
    char buffer[BUFFER_SIZE];
    sockaddr_in senderAddr;
    socklen_t senderLen = sizeof(senderAddr);

    while (running) {
        std::memset(buffer, 0, BUFFER_SIZE);
        int received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, (sockaddr*)&senderAddr, &senderLen);

        if (received > 0) {
            buffer[received] = '\0'; // Null-terminate for safety
            std::cout << "[BEACON RECEIVED] From " << inet_ntoa(senderAddr.sin_addr)
                      << ":" << ntohs(senderAddr.sin_port) << " - Message: \"" << buffer << "\"\n";
        }
    }

    // Cleanup
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    std::cout << "[INFO] Receiver shutting down.\n";
    return 0;
}
