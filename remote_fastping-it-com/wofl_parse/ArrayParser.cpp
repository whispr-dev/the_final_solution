#include "ArrayParser.hpp"

bool ArrayParser::parse(std::vector<std::string>& out) {
    Token t = tokenizer.next();
    if (t.type != TokenType::ArrayStart) return false;

    while (true) {
        Token val = tokenizer.next();
        if (val.type == TokenType::ArrayEnd) break;
        if (val.type != TokenType::String) return false;

        out.emplace_back(val.value);

        Token next = tokenizer.next();
        if (next.type == TokenType::ArrayEnd) break;
        if (next.type != TokenType::Comma) return false;
    }
    return true;
}
