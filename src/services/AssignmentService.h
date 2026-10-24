#pragma once
#include "data/Database.h"
#include "data/IAssignmentRepository.h"
#include "data/ICourseRepository.h"
#include "data/ISubmissionRepository.h"
#include "data/IUserRepository.h"
#include "services/IClock.h"
#include "services/IPenaltyPolicy.h"

#include <string>

namespace lms {

// Lecturer functions for assignments and extensions (UC-06, UC-07; FR-07, FR-08).
// Each method takes the lecturer's id and checks they teach the course (FR-02).
class AssignmentService {
public:
    AssignmentService(Database& db, IAssignmentRepository& assignments,
                      ICourseRepository& courses, IUserRepository& users,
                      ISubmissionRepository& submissions, const IPenaltyPolicy& policy,
                      const IClock& clock);

    // FR-07. The deadline must be in the future and the max mark 1 to 100.
    int createAssignment(int lecturerId, int courseId, const std::string& title,
                         TimePoint deadline, int maxMark, const std::string& description = "");

    // FR-08, BR-03. The new deadline must be later than the normal deadline and
    // at most 14 days after it, and the reason must not be empty. Refused once the
    // student's work is marked. If the student has already submitted, days late is
    // worked out again against the new deadline (BR-04).
    void grantExtension(int lecturerId, int assignmentId, int studentId, TimePoint newDeadline,
                        const std::string& reason);

private:
    void requireLecturerOf(int lecturerId, int courseId);
    Assignment requireAssignment(int assignmentId);

    Database& db_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    IUserRepository& users_;
    ISubmissionRepository& submissions_;
    const IPenaltyPolicy& policy_;
    const IClock& clock_;
};

}  // namespace lms
