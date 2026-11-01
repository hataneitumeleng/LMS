#pragma once
#include "domain/Assignment.h"
#include "domain/Extension.h"

#include <optional>
#include <vector>

namespace lms {

// Assignments and extensions (FR-07, FR-08).
class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual std::optional<Assignment> find(int id) = 0;
    virtual std::vector<Assignment> forCourse(int courseId) = 0;
    // Insert when id is 0, otherwise update. Return the id.
    virtual int save(const Assignment& assignment) = 0;

    virtual std::optional<Extension> findExtension(int assignmentId, int studentId) = 0;
    // A second extension for the same student replaces the first.
    virtual void saveExtension(const Extension& extension) = 0;

    // Every stored brief file, so start-up clean-up keeps them (FR-24).
    virtual std::vector<std::string> allBriefPaths() = 0;
};

}  // namespace lms
