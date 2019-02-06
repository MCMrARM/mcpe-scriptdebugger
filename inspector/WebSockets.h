#pragma once

#include <string>
#include <cstdlib>
#include "../minecraft/Crypto.h"
#include "../minecraft/Util.h"

#define SIMPLE_WEB_CRYPTO_HPP
namespace SimpleWeb {
    class Crypto {
    public:
        static std::string sha1(const std::string &input) noexcept {
            return ::Crypto::Hash::hash(::Crypto::Hash::HashType::SHA1, input);
        }

        class Base64 {
        public:
            static std::string encode(std::string const& s) {
                return Util::base64_encode((unsigned char*) s.c_str(), s.size(), true);
            }
        };
    };
}

#define ASIO_STANDALONE
#define USE_STANDALONE_ASIO
#include <server_ws.hpp>