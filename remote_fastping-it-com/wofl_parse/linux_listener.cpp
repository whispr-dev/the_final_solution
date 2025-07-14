// linux_listener.cpp - Linux version of your listener
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <iomanip>

void print_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
}

int main() {
    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9876);  // Same port as your beacon
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        close(sock);
        return 1;
    }

    std::cout << "🚀 FastPing UDP Listener started on port 9876\n";
    std::cout << "📡 Waiting for beacon signals...\n\n";

    char buffer[1024];
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (true) {
        int recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
                               (sockaddr*)&client_addr, &addr_len);
        
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            
            // Get client IP
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            
            // Print with timestamp
            print_timestamp();
            std::cout << "📨 FROM " << client_ip << ":" << ntohs(client_addr.sin_port) 
                     << " | DATA: " << buffer << std::endl;
        }
    }

    close(sock);
    return 0;
}