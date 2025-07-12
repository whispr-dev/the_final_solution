#pragma once
#include "Tokenizer.hpp"
#include <vector>
#include <string>

class ArrayParser {
public:
    ArrayParser(std::string_view src) : tokenizer(src) {}

    bool parse(std::vector<std::string>& out);

private:
    Tokenizer tokenizer;
};
