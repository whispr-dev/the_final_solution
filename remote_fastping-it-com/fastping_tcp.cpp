// fastping_tcp.cpp
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
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define TARGET_IP "8.8.8.8"
#define TARGET_PORT 80
#define TIMEOUT_SEC 2

std::atomic<bool> running(true);

void signalHandler(int signum) {
    std::cout << "\n[INFO] Signal " << signum << " received. Exiting.\n";
    running = false;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    std::signal(SIGINT, signalHandler);

    std::cout << "[INFO] TCP Ping to " << TARGET_IP << ":" << TARGET_PORT << "\n";

    while (running) {
        int sock;
#ifdef _WIN32
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
        sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
        if (sock < 0) {
            std::cerr << "[ERROR] Failed to create socket.\n";
            break;
        }

        sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(TARGET_PORT);
#ifdef _WIN32
        inet_pton(AF_INET, TARGET_IP, &target.sin_addr);
#else
        inet_aton(TARGET_IP, &target.sin_addr);
#endif

        auto start = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
        u_long nonBlocking = 1;
        ioctlsocket(sock, FIONBIO, &nonBlocking);
#else
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

        connect(sock, (sockaddr*)&target, sizeof(target));

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(sock, &writeSet);
        timeval timeout = { TIMEOUT_SEC, 0 };

        if (select(sock + 1, nullptr, &writeSet, nullptr, &timeout) > 0) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "[OPEN] Port " << TARGET_PORT << " reachable in " << duration.count() << "ms\n";
        } else {
            std::cout << "[CLOSED] No response.\n";
        }

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
