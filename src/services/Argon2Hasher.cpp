#include "services/Argon2Hasher.h"

#include <sodium.h>
#include <stdexcept>

namespace lms {

Argon2Hasher::Argon2Hasher() {
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium could not be started");
    }
}

std::string Argon2Hasher::hash(const std::string& password) const {
    char result[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(result, password.c_str(), password.size(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed (out of memory)");
    }
    return result;
}

bool Argon2Hasher::verify(const std::string& password, const std::string& hash) const {
    return crypto_pwhash_str_verify(hash.c_str(), password.c_str(), password.size()) == 0;
}

}  // namespace lms
