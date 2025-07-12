#pragma once
#include <tuple>
#include <string>
#include <utility>

template<typename T>
struct Reflector;
#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>

enum class TokenType {
    ObjectStart,
    ObjectEnd,
    String,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
};

class Tokenizer {
    // Your tokenizer logic
};

#endif

struct PingResponse {
    std::string anonymity_level;
    std::string args_raw;
    std::string client_ip_from_headers;
    std::string message;
    std::string status;
};

template<>
struct Reflector<PingResponse> {
    static constexpr auto fields = std::make_tuple(
        std::make_pair("anonymity_level", &PingResponse::anonymity_level),
        std::make_pair("args", &PingResponse::args_raw),
        std::make_pair("client_ip_from_headers", &PingResponse::client_ip_from_headers),
        std::make_pair("message", &PingResponse::message),
        std::make_pair("status", &PingResponse::status)
    );
};
