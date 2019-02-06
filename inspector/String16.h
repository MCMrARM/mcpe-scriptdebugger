#pragma once

#include <string>

namespace v8_inspector {

    class StringView;

    struct String16 {
        std::basic_string<uint16_t> impl;
        std::size_t hash_code = 0;

        std::string utf8() const;
    };

    String16 toString16(StringView const&);

}