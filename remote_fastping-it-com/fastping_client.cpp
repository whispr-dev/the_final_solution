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
    std::cout << "\n[INFO] Caught signal " << signum << ", exiting.\n";
    running = false;
}

void printUsage() {
    std::cout << "Usage: fastping_client --protocol tcp|udp|icmp --target <ip> [--port <port>] [--format text|json]\n";
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
            std::cout << "[" << protocol << "] " << target << ":" << port << " reachable in " << ms << "ms\n";
        } else {
            std::cout << "[" << protocol << "] " << target << ":" << port << " unreachable.\n";
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

int main(int argc, char* argv[]) {
    std::string protocol, target, format = "text";
    int port = 80;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    std::signal(SIGINT, signalHandler);

    // Simple CLI parse
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

    if (protocol != "tcp") {
        std::cerr << "[ERROR] Only TCP implemented in this artifact. More protocols next build.\n";
        return 1;
    }

    std::cout << "[INFO] Running " << protocol << " ping to " << target << ":" << port << "\n";

    while (running) {
        int latency = tcpPing(target, port);
        bool success = latency >= 0;
        outputResult(protocol, target, port, success, latency, format);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
