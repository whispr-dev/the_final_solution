#include <iostream>
#include <string>
#include <unordered_map>
#include <any>
#include <winsock2.h>
#include "tokenizer.hpp"
#include "Parser.hpp"

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(1337);
    local.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (sockaddr*)&local, sizeof(local));

    char buffer[1024];
    sockaddr_in sender;
    int senderSize = sizeof(sender);

    while (true) {
        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&sender, &senderSize);
        if (len <= 0) continue;

        buffer[len] = '\0';
        std::string jsonStr(buffer, len);

        std::cout << "Received JSON: " << jsonStr << "\n";

        Tokenizer tokenizer(jsonStr);
        Parser<std::unordered_map<std::string, std::any>> parser(tokenizer);

        std::unordered_map<std::string, std::any> result;
        if (parser.parse(result)) {
            std::cout << "Parsed OK:\n";
            for (const auto& [key, value] : result) {
                if (value.type() == typeid(std::string))
                    std::cout << key << " : " << std::any_cast<std::string>(value) << "\n";
            }
        } else {
            std::cerr << "Failed to parse JSON.\n";
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
