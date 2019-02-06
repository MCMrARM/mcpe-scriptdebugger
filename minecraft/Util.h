#pragma once

#include <string>

class Util {
public:
    static std::string base64_encode(unsigned char const*, unsigned int, bool);
};