// Unit tests for the real password hasher (NFR-03): libsodium Argon2id with a random salt.
#include "services/Argon2Hasher.h"

#include <gtest/gtest.h>

namespace lms::test {
namespace {

TEST(Argon2HasherTest, HashIsArgon2idAndNotThePassword) {
    const Argon2Hasher hasher;
    const std::string hash = hasher.hash("Password1");
    EXPECT_EQ(hash.rfind("$argon2id$", 0), 0u);  // the algorithm is named in the hash
    EXPECT_EQ(hash.find("Password1"), std::string::npos);
}

TEST(Argon2HasherTest, SamePasswordTwiceGivesDifferentHashes) {  // a new salt each time
    const Argon2Hasher hasher;
    EXPECT_NE(hasher.hash("Password1"), hasher.hash("Password1"));
}

TEST(Argon2HasherTest, VerifyAcceptsTheRightPasswordOnly) {
    const Argon2Hasher hasher;
    const std::string hash = hasher.hash("Password1");
    EXPECT_TRUE(hasher.verify("Password1", hash));
    EXPECT_FALSE(hasher.verify("password1", hash));
    EXPECT_FALSE(hasher.verify("", hash));
}

}  // namespace
}  // namespace lms::test
