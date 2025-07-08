#include "Parser.hpp"
#include "Reflector.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>

// Minifier from your AVX2 setup
extern "C" size_t minify_json_avx2(const char* src, size_t len, char* dst);

// Struct for expected JSON fields
struct PingResponse {
    std::string anonymity_level;
    std::string client_ip_from_headers;
    std::string message;
    std::string status;
};

// Reflector setup for parsing fields
template<>
struct Reflector<PingResponse> {
    static constexpr auto fields = std::make_tuple(
        std::make_pair("anonymity_level", &PingResponse::anonymity_level),
        std::make_pair("client_ip_from_headers", &PingResponse::client_ip_from_headers),
        std::make_pair("message", &PingResponse::message),
        std::make_pair("status", &PingResponse::status)
    );
};

// Run shell command and capture output
std::string runCommand(const char* cmd) {
    char buffer[256];
    std::string result;
    FILE* pipe = _popen(cmd, "r");
    if (!pipe) return "";
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    _pclose(pipe);
    return result;
}

int main() {
    while (true) {
        std::string json = runCommand("curl -s -L http://161.35.248.233:9876/ping");

        // Prepare buffer for minified output
        std::string minified(json.size(), '\0');
        size_t new_len = minify_json_avx2(json.data(), json.size(), minified.data());
        minified.resize(new_len);

        PingResponse response;
        Parser<PingResponse> parser(minified);

        if (parser.parse(response)) {
            std::cout << "=== Parsed Beacon Response ===\n";
            std::cout << "Anonymity Level: " << response.anonymity_level << "\n";
            std::cout << "Client IP: " << response.client_ip_from_headers << "\n";
            std::cout << "Message: " << response.message << "\n";
            std::cout << "Status: " << response.status << "\n";
        } else {
            std::cout << "Failed to parse JSON response!\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
