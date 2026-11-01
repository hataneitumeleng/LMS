#pragma once
#include "FakeAssignmentRepository.h"
#include "FakeCourseRepository.h"
#include "FakeSubmissionRepository.h"
#include "data/IAuditRepository.h"

#include <vector>

namespace lms::test {

// IAuditRepository kept in memory. To find a department's entries it follows
// submission -> assignment -> course -> department through the other fakes, the same
// joins the real SQL query makes.
class FakeAuditRepository : public IAuditRepository {
public:
    FakeAuditRepository(FakeSubmissionRepository& submissions,
                        FakeAssignmentRepository& assignments, FakeCourseRepository& courses)
        : submissions_(submissions), assignments_(assignments), courses_(courses) {}

    void add(const AuditEntry& entry) override {
        AuditEntry e = entry;
        e.id = static_cast<int>(entries_.size()) + 1;
        entries_.push_back(e);
    }
    std::vector<AuditEntry> findByDepartment(int departmentId) override {
        std::vector<AuditEntry> result;
        for (const AuditEntry& e : entries_) {
            const auto submission = submissions_.findById(e.submissionId);
            if (!submission) continue;
            const auto assignment = assignments_.find(submission->assignmentId);
            if (!assignment) continue;
            const auto course = courses_.findCourse(assignment->courseId);
            if (course && course->departmentId == departmentId) result.push_back(e);
        }
        return result;
    }
    std::vector<AuditEntry> findAll() override { return entries_; }

private:
    FakeSubmissionRepository& submissions_;
    FakeAssignmentRepository& assignments_;
    FakeCourseRepository& courses_;
    std::vector<AuditEntry> entries_;
};

}  // namespace lms::test
