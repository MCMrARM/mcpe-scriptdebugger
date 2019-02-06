#pragma once

#include <string>

namespace Crypto {
    namespace Hash {
        enum class HashType {
            SHA1 = 1
        };
        std::string hash(HashType, std::string const&);
    }
}
