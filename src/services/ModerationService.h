#pragma once
#include "data/Database.h"
#include "data/IAssignmentRepository.h"
#include "data/ICourseRepository.h"
#include "data/ISubmissionRepository.h"
#include "data/IUserRepository.h"
#include "services/AuditService.h"
#include "services/IClock.h"

#include <string>
#include <vector>

namespace lms {

// One assignment in the HoD's department, with how much is still to be moderated.
struct DepartmentAssignment {
    std::string courseCode;
    Assignment assignment;
    int submissions = 0;
    int notModerated = 0;
};

// HoD moderation and release (UC-12, UC-13; FR-14, FR-15, BR-08), Figure 7.
// Every method checks that the assignment belongs to the HoD's own department.
class ModerationService {
public:
    ModerationService(Database& db, ISubmissionRepository& submissions,
                      IAssignmentRepository& assignments, ICourseRepository& courses,
                      IUserRepository& users, AuditService& audit, const IClock& clock);

    // Every assignment of every course in the HoD's department.
    std::vector<DepartmentAssignment> assignmentsInDepartment(int hodId);

    // The marked work of one assignment (Marked, Moderated or Released).
    std::vector<Submission> listForModeration(int hodId, int assignmentId);

    // FR-14. Every Marked submission becomes Moderated with its mark unchanged.
    // No audit entry, because no mark changes (FR-17). Returns how many were approved.
    int approveAll(int hodId, int assignmentId);

    // FR-14, BR-08. Sets a new final mark with a reason, and records it in the audit
    // trail in the same transaction (NFR-08). Also allowed after release.
    void adjust(int hodId, int submissionId, double finalMark, const std::string& reason);

    // FR-15. Refused unless every submission is Moderated. Returns how many were released.
    int release(int hodId, int assignmentId);

    // The latest version of a student's work, so the HoD can check it while moderating.
    // Only for an assignment in the HoD's own department (FR-02).
    SubmissionVersion latestFile(int hodId, int submissionId);

private:
    int requireHodDepartment(int hodId);
    Assignment requireAssignmentInDepartment(int hodId, int assignmentId);

    Database& db_;
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    IUserRepository& users_;
    AuditService& audit_;
    const IClock& clock_;
};

}  // namespace lms
