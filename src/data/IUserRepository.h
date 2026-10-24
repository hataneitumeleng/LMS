#pragma once
#include "domain/User.h"

#include <memory>
#include <string>
#include <vector>

namespace lms {

// Repository pattern (Table 1): the services ask for users through this interface
// and never see SQL. Tests can replace it with an in-memory fake.
class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    // Each find returns nullptr if there is no such user.
    virtual std::unique_ptr<User> findByUsername(const std::string& username) = 0;
    virtual std::unique_ptr<User> findById(int id) = 0;
    virtual std::unique_ptr<User> findActiveHod(int departmentId) = 0;
    virtual std::vector<std::unique_ptr<User>> findAll() = 0;
    // Inserts a new user (id 0) or updates an existing one. Returns the id.
    virtual int save(const User& user) = 0;
};

}  // namespace lms
