#pragma once
#include "data/Database.h"
#include "data/IUserRepository.h"
#include "domain/UserFactory.h"

namespace lms {

// Stores users in the user table (single-table inheritance, Section 5.2).
class SqliteUserRepository : public IUserRepository {
public:
    SqliteUserRepository(Database& db, const UserFactory& factory);

    std::unique_ptr<User> findByUsername(const std::string& username) override;
    std::unique_ptr<User> findById(int id) override;
    std::unique_ptr<User> findActiveHod(int departmentId) override;
    std::vector<std::unique_ptr<User>> findAll() override;
    int save(const User& user) override;

private:
    std::unique_ptr<User> readUser(Statement& row) const;

    Database& db_;
    const UserFactory& factory_;
};

}  // namespace lms
