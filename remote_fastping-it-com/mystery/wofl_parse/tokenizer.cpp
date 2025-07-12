#include "tokenizer.hpp"
#include <cctype>

Tokenizer::Tokenizer(const std::string& input) : src(input) {}

void Tokenizer::skipWhitespace() {
    while (pos < src.size() && isspace(src[pos])) {
        ++pos;
    }
}

Token Tokenizer::next() {
    skipWhitespace();
    if (pos >= src.size()) return {TokenType::Unknown, ""};

    if (src[pos] == '{') {
        ++pos;
        return {TokenType::ObjectStart, "{"};
    }
    if (src[pos] == '}') {
        ++pos;
        return {TokenType::ObjectEnd, "}"};
    }
    if (src[pos] == '"') {
        return parseString();
    }

    return {TokenType::Unknown, std::string(1, src[pos++])};
}

Token Tokenizer::parseString() {
    ++pos;  // Skip opening quote
    size_t start = pos;
    while (pos < src.size() && src[pos] != '"') ++pos;
    std::string str = src.substr(start, pos - start);
    ++pos;  // Skip closing quote
    return {TokenType::String, str};
}
