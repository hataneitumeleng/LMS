#pragma once
#include "data/Database.h"
#include "data/IAssignmentRepository.h"

namespace lms {

// Stores the assignment and extension tables.
class SqliteAssignmentRepository : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(Database& db);

    std::optional<Assignment> find(int id) override;
    std::vector<Assignment> forCourse(int courseId) override;
    int save(const Assignment& assignment) override;

    std::optional<Extension> findExtension(int assignmentId, int studentId) override;
    void saveExtension(const Extension& extension) override;

private:
    static Assignment readAssignment(const Statement& row);

    Database& db_;
};

}  // namespace lms
