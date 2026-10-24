#pragma once
#include "services/IPasswordHasher.h"

namespace lms {

// Argon2id from libsodium (NFR-03). Argon2 won the Password Hashing Competition and
// needs a lot of memory per guess, which makes brute-force attacks slow. Each hash
// contains its own random salt, so two equal passwords give different hashes.
class Argon2Hasher : public IPasswordHasher {
public:
    Argon2Hasher();  // starts libsodium; throws if it cannot
    std::string hash(const std::string& password) const override;
    bool verify(const std::string& password, const std::string& hash) const override;
};

}  // namespace lms
