#pragma once
#include <string>

namespace lms {

// Turns a password into a hash and checks a password against a hash (NFR-03).
class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    virtual std::string hash(const std::string& password) const = 0;
    virtual bool verify(const std::string& password, const std::string& hash) const = 0;
};

}  // namespace lms
