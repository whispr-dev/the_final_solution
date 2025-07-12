#include "Parser.hpp"

template<typename T>
bool Parser<T>::parse(T& out) {
    Token t = tokenizer.next();
    if (t.type != TokenType::ObjectStart) return false;
    return parseObject(out);
}

template<typename T>
bool Parser<T>::parseObject(T& out) {
    while (true) {
        Token key = tokenizer.next();
        if (key.type == TokenType::ObjectEnd) break;
        if (key.type != TokenType::String) return false;

        Token colon = tokenizer.next();
        if (colon.type != TokenType::Colon) return false;

        Token value = tokenizer.next();

        bool matched = false;

        std::apply([&](auto&&... field) {
            ((key.value == field.first && (
                [&] {
                    if constexpr (std::is_same_v<decltype(out.*(field.second)), std::string>) {
                        if (value.type == TokenType::String)
                            out.*(field.second) = std::string(value.value);
                    } else if constexpr (std::is_integral_v<decltype(out.*(field.second))>) {
                        if (value.type == TokenType::Number)
                            out.*(field.second) = std::stoi(std::string(value.value));
                    }
                    matched = true;
                }()
            )), ...);
        }, Reflector<T>::fields);

        if (!matched) {
            std::cerr << "Unknown field: " << key.value << "\n";
        }

        Token next = tokenizer.next();
        if (next.type == TokenType::ObjectEnd) break;
        if (next.type != TokenType::Comma) return false;
    }
    return true;
}
