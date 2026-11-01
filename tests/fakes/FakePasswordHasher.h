#pragma once
#include "services/IPasswordHasher.h"

#include <string>

namespace lms::test {

// A very fast stand-in for Argon2Hasher. Argon2id is slow on purpose (NFR-03), which would
// make hundreds of login tests slow. The real hasher has its own tests (Argon2HasherTest).
class FakePasswordHasher : public IPasswordHasher {
public:
    std::string hash(const std::string& password) const override { return "hash:" + password; }
    bool verify(const std::string& password, const std::string& hash) const override {
        return hash == "hash:" + password;
    }
};

}  // namespace lms::test
