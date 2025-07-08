// File: linux_beacon_sender.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// Configuration
const char* BASE_IP = "192.168.1.100";    // Starting IP address
const int TARGET_PORT = 9000;             // Target port
const int INTERVAL_MS = 10000;            // 10 seconds between pings
const int IP_COUNT = 10;                  // Number of sequential IPs to cycle through

class LinuxBeaconSender {
private:
    int sock;
    std::vector<std::string> target_ips;
    int current_ip_index;
    
public:
    LinuxBeaconSender() : current_ip_index(0), sock(-1) {
        // Create socket
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        
        // Generate sequential IP addresses
        generate_ip_list();
    }
    
    ~LinuxBeaconSender() {
        if (sock >= 0) {
            close(sock);
        }
    }
    
    void generate_ip_list() {
        // Parse base IP and generate sequential IPs
        std::string base_ip = BASE_IP;
        size_t last_dot = base_ip.find_last_of('.');
        
        if (last_dot == std::string::npos) {
            throw std::runtime_error("Invalid base IP format");
        }
        
        std::string ip_prefix = base_ip.substr(0, last_dot + 1);
        int last_octet = std::stoi(base_ip.substr(last_dot + 1));
        
        target_ips.clear();
        for (int i = 0; i < IP_COUNT; ++i) {
            int new_octet = last_octet + i;
            if (new_octet > 255) {
                new_octet = new_octet - 256; // Wrap around if needed
            }
            target_ips.push_back(ip_prefix + std::to_string(new_octet));
        }
        
        // Print the IP range we'll be cycling through
        std::cout << "🎯 Beacon will cycle through IPs:\n";
        for (size_t i = 0; i < target_ips.size(); ++i) {
            std::cout << "   " << (i + 1) << ": " << target_ips[i] << ":" << TARGET_PORT << "\n";
        }
        std::cout << "\n";
    }
    
    std::string create_beacon_message() {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();
        
        // Create JSON beacon with useful info
        std::string message = "{"
            "\"type\":\"BEACON_PING\","
            "\"timestamp\":" + std::to_string(timestamp) + ","
            "\"sequence\":" + std::to_string(current_ip_index + 1) + ","
            "\"target_ip\":\"" + target_ips[current_ip_index] + "\","
            "\"total_targets\":" + std::to_string(IP_COUNT) + ","
            "\"cycle_complete\":" + (current_ip_index == IP_COUNT - 1 ? "true" : "false") +
            "}";
        
        return message;
    }
    
    bool send_beacon() {
        std::string current_ip = target_ips[current_ip_index];
        std::string beacon_msg = create_beacon_message();
        
        struct sockaddr_in dest;
        memset(&dest, 0, sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_port = htons(TARGET_PORT);
        
        if (inet_pton(AF_INET, current_ip.c_str(), &dest.sin_addr) <= 0) {
            std::cerr << "❌ Invalid IP address: " << current_ip << "\n";
            return false;
        }
        
        ssize_t sent = sendto(sock, beacon_msg.c_str(), beacon_msg.length(), 0, 
                             (struct sockaddr*)&dest, sizeof(dest));
        
        if (sent < 0) {
            std::cerr << "❌ Send failed to " << current_ip << ":" << TARGET_PORT 
                      << " (Error: " << strerror(errno) << ")\n";
            return false;
        } else {
            std::cout << "🚀 Beacon [" << (current_ip_index + 1) << "/" << IP_COUNT 
                      << "] sent to " << current_ip << ":" << TARGET_PORT;
            
            // Add cycle completion indicator
            if (current_ip_index == IP_COUNT - 1) {
                std::cout << " ✅ CYCLE COMPLETE";
            }
            std::cout << "\n";
            return true;
        }
    }
    
    void run() {
        std::cout << "🎯 Starting Linux UDP Beacon Sender\n";
        std::cout << "📡 Interval: " << INTERVAL_MS << "ms\n";
        std::cout << "🔄 Cycling through " << IP_COUNT << " sequential IPs\n\n";
        
        int total_sent = 0;
        int cycles_completed = 0;
        
        while (true) {
            if (send_beacon()) {
                total_sent++;
            }
            
            // Move to next IP in sequence
            current_ip_index = (current_ip_index + 1) % IP_COUNT;
            
            // Track completed cycles
            if (current_ip_index == 0) {
                cycles_completed++;
                std::cout << "🔄 Completed cycle #" << cycles_completed 
                          << " (Total beacons sent: " << total_sent << ")\n\n";
            }
            
            // Sleep until next beacon
            std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_MS));
        }
    }
};

// Signal handler for clean shutdown
#include <signal.h>
bool running = true;

void signal_handler(int signal) {
    std::cout << "\n🛑 Received signal " << signal << ", shutting down gracefully...\n";
    running = false;
}

int main() {
    // Set up signal handling for clean shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        LinuxBeaconSender beacon;
        beacon.run();
    }
    catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}