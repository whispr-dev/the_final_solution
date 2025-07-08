// fastping_client.cpp
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

std::atomic<bool> running(true);

void signalHandler(int signum) {
    std::cout << "\n[INFO] Signal " << signum << " received. Exiting.\n";
    running = false;
}

void printUsage() {
    std::cout << "Usage: fastping_client --protocol icmp|udp|tcp --target <ip> [--port <port>] [--format text|json]\n";
}

void outputResult(const std::string& protocol, const std::string& target, int port, bool success, int ms, const std::string& format) {
    if (format == "json") {
        std::cout << "{ \"protocol\": \"" << protocol
                  << "\", \"target\": \"" << target
                  << "\", \"port\": " << port
                  << ", \"success\": " << (success ? "true" : "false")
                  << ", \"latency_ms\": " << ms
                  << " }\n";
    } else {
        if (success) {
            std::cout << "[" << protocol << "] " << target;
            if (port > 0) std::cout << ":" << port;
            std::cout << " reachable in " << ms << "ms\n";
        } else {
            std::cout << "[" << protocol << "] " << target;
            if (port > 0) std::cout << ":" << port;
            std::cout << " unreachable.\n";
        }
    }
}

int tcpPing(const std::string& target, int port) {
    auto start = std::chrono::high_resolution_clock::now();

    int sock;
#ifdef _WIN32
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (sock < 0) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
    inet_pton(AF_INET, target.c_str(), &addr.sin_addr);
#else
    inet_aton(target.c_str(), &addr.sin_addr);
#endif

#ifdef _WIN32
    u_long nonBlocking = 1;
    ioctlsocket(sock, FIONBIO, &nonBlocking);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

    connect(sock, (sockaddr*)&addr, sizeof(addr));

    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(sock, &writeSet);
    timeval timeout = { 2, 0 };

    int result = select(sock + 1, nullptr, &writeSet, nullptr, &timeout);
    int ms = -1;

    if (result > 0) {
        auto end = std::chrono::high_resolution_clock::now();
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    return ms;
}

int udpPing(const std::string& target, int port) {
    auto start = std::chrono::high_resolution_clock::now();

    int sock;
#ifdef _WIN32
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if (sock < 0) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
    inet_pton(AF_INET, target.c_str(), &addr.sin_addr);
#else
    inet_aton(target.c_str(), &addr.sin_addr);
#endif

    const char* msg = "fastping_test";
    sendto(sock, msg, strlen(msg), 0, (sockaddr*)&addr, sizeof(addr));

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);
    timeval timeout = { 2, 0 };

    int ms = -1;
    char buffer[512];
    sockaddr_in from;
    socklen_t fromLen = sizeof(from);

    if (select(sock + 1, &readSet, nullptr, nullptr, &timeout) > 0) {
        int received = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
        if (received > 0) {
            auto end = std::chrono::high_resolution_clock::now();
            ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }
    }

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    return ms;
}

int icmpPing(const std::string& target) {
#ifdef _WIN32
    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == INVALID_SOCKET) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, target.c_str(), &addr.sin_addr);

    struct {
        BYTE type;
        BYTE code;
        USHORT checksum;
        USHORT id;
        USHORT seq;
    } icmpHeader = { 8, 0, 0, (USHORT)GetCurrentProcessId(), 0 };

    icmpHeader.checksum = ~((icmpHeader.type << 8) + icmpHeader.code);

    char packet[sizeof(icmpHeader)];
    memcpy(packet, &icmpHeader, sizeof(icmpHeader));

    auto start = std::chrono::high_resolution_clock::now();

    sendto(sock, packet, sizeof(packet), 0, (sockaddr*)&addr, sizeof(addr));

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);
    timeval timeout = { 2, 0 };

    char buffer[512];
    sockaddr_in from;
    int fromLen = sizeof(from);
    int ms = -1;

    if (select(0, &readSet, nullptr, nullptr, &timeout) > 0) {
        int received = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
        if (received > 0) {
            auto end = std::chrono::high_resolution_clock::now();
            ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }
    }

    closesocket(sock);
    return ms;
#else
    std::cerr << "[ERROR] ICMP requires raw sockets. Not implemented for Linux in this demo.\n";
    return -1;
#endif
}

int main(int argc, char* argv[]) {
    std::string protocol, target, format = "text";
    int port = 80;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    std::signal(SIGINT, signalHandler);

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--protocol" && i + 1 < argc) protocol = argv[++i];
        else if (arg == "--target" && i + 1 < argc) target = argv[++i];
        else if (arg == "--port" && i + 1 < argc) port = std::stoi(argv[++i]);
        else if (arg == "--format" && i + 1 < argc) format = argv[++i];
    }

    if (protocol.empty() || target.empty()) {
        printUsage();
        return 1;
    }

    std::cout << "[INFO] Running " << protocol << " ping to " << target;
    if (protocol != "icmp") std::cout << ":" << port;
    std::cout << "\n";

    while (running) {
        int latency = -1;

        if (protocol == "tcp") latency = tcpPing(target, port);
        else if (protocol == "udp") latency = udpPing(target, port);
        else if (protocol == "icmp") latency = icmpPing(target);
        else {
            std::cerr << "[ERROR] Unknown protocol.\n";
            break;
        }

        bool success = latency >= 0;
        outputResult(protocol, target, port, success, latency, format);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
