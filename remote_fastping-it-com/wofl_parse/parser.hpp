#ifndef PARSER_HPP
#define PARSER_HPP

#include "Tokenizer.hpp"
#include <functional>

template<typename T>
class Parser {
public:
    explicit Parser(Tokenizer& tokenizer) : tokenizer(tokenizer) {}

    bool parse(T& out) {
        Token t = tokenizer.next();
        if (t.type != TokenType::ObjectStart) return false;
        return parseObject(out);
    }

private:
    Tokenizer& tokenizer;

    bool parseObject(T& out) {
        while (true) {
            Token key = tokenizer.next();
            if (key.type == TokenType::ObjectEnd) break;
            Token value = tokenizer.next();

            if (value.type == TokenType::String) {
                out[key.value] = value.value;
            } else if (value.type == TokenType::ObjectStart) {
                T child;
                if (parseObject(child)) {
                    out[key.value] = child;
                }
            }
        }
        return true;
    }
};

#endif // PARSER_HPP
