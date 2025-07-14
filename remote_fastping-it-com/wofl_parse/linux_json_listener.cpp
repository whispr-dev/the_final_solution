// linux_json_listener.cpp - Enhanced JSON listener with monitoring
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <json/json.h>  // Install: sudo apt install libjsoncpp-dev

class BeaconMonitor {
private:
    std::ofstream log_file;
    int packet_count = 0;
    
    void print_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
    }
    
    void log_to_file(const std::string& message) {
        if (log_file.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            log_file << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] " 
                     << message << std::endl;
            log_file.flush();
        }
    }

public:
    BeaconMonitor() {
        log_file.open("beacon_monitor.log", std::ios::app);
        
        std::cout << "🔥 FastPing Beacon Monitor v1.0\n";
        std::cout << "================================\n";
        std::cout << "📡 Listening on port 9876 for UDP beacons\n";
        std::cout << "📝 Logging to beacon_monitor.log\n";
        std::cout << "🚀 Press Ctrl+C to stop\n\n";
    }
    
    ~BeaconMonitor() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }
    
    void process_packet(const std::string& data, const std::string& client_ip, int client_port) {
        packet_count++;
        
        print_timestamp();
        std::cout << "📦 PKT#" << packet_count << " FROM " << client_ip << ":" << client_port << std::endl;
        
        // Try to parse as JSON
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(data, root)) {
            std::cout << "✅ VALID JSON:" << std::endl;
            
            // Extract common beacon fields
            if (root.isMember("status")) {
                std::cout << "   🚨 Status: " << root["status"].asString() << std::endl;
            }
            if (root.isMember("id")) {
                std::cout << "   🏷️  ID: " << root["id"].asString() << std::endl;
            }
            if (root.isMember("time")) {
                std::cout << "   ⏰ Time: " << root["time"].asString() << std::endl;
            }
            
            // Log structured data
            std::string log_msg = "JSON_BEACON from " + client_ip + " | " + data;
            log_to_file(log_msg);
            
        } else {
            std::cout << "📄 RAW DATA: " << data << std::endl;
            
            // Log raw data
            std::string log_msg = "RAW_DATA from " + client_ip + " | " + data;
            log_to_file(log_msg);
        }
        
        std::cout << "───────────────────────────────────────\n" << std::endl;
    }
    
    void start_listening() {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            std::cerr << "❌ Socket creation failed\n";
            return;
        }

        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(9876);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "❌ Bind failed\n";
            close(sock);
            return;
        }

        char buffer[1024];
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        while (true) {
            int recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
                                   (sockaddr*)&client_addr, &addr_len);
            
            if (recv_len > 0) {
                buffer[recv_len] = '\0';
                
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                
                process_packet(std::string(buffer), std::string(client_ip), 
                              ntohs(client_addr.sin_port));
            }
        }

        close(sock);
    }
};

int main() {
    BeaconMonitor monitor;
    monitor.start_listening();
    return 0;
}