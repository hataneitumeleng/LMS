#pragma once
#include "data/IAssignmentRepository.h"
#include "data/ICourseRepository.h"
#include "data/IUserRepository.h"
#include "services/IClock.h"

#include <string>

namespace lms {

// Lecturer functions for assignments and extensions (UC-06, UC-07; FR-07, FR-08).
// Each method takes the lecturer's id and checks they teach the course (FR-02).
class AssignmentService {
public:
    AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      IUserRepository& users, const IClock& clock);

    // FR-07. The deadline must be in the future and the max mark 1 to 100.
    int createAssignment(int lecturerId, int courseId, const std::string& title,
                         TimePoint deadline, int maxMark, const std::string& description = "");

    // FR-08, BR-03. The new deadline must be later than the normal deadline and
    // at most 14 days after it, and the reason must not be empty.
    void grantExtension(int lecturerId, int assignmentId, int studentId, TimePoint newDeadline,
                        const std::string& reason);

private:
    void requireLecturerOf(int lecturerId, int courseId);
    Assignment requireAssignment(int assignmentId);

    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    IUserRepository& users_;
    const IClock& clock_;
};

}  // namespace lms
