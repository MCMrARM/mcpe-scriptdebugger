#pragma once

#include <cstdlib>
#include <cstdio>
#include <boost/lexical_cast.hpp>

#define localeconv localeconv_polyfill

extern struct lconv *localeconv_polyfill();

namespace std {

template <typename T>
static std::string to_string(T value) {
    return boost::lexical_cast<std::string>(value);
}

static float strtof(const char* str, char** str_end) {
    return (float) strtod(str, str_end);
}
static long double strtold(const char* str, char** str_end) {
    return strtod(str, str_end);
}

using ::strtoll;
using ::strtoull;
using ::snprintf;

static int stoi(const std::string& str, size_t* idx = nullptr, int base = 10) {
    char* endp = nullptr;
    int ret = (int) strtol(str.c_str(), &endp, base);
    if (idx)
        *idx = endp - str.c_str();
    return ret;
}

using ::localeconv_polyfill;

}