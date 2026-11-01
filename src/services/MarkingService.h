#pragma once
#include "data/Database.h"
#include "data/IAssignmentRepository.h"
#include "data/ICourseRepository.h"
#include "data/ISubmissionRepository.h"
#include "services/AuditService.h"
#include "services/IPenaltyPolicy.h"

#include <string>
#include <vector>

namespace lms {

// Lecturer marking (UC-08; FR-12, FR-13, FR-18).
// Every mark change and its audit entry are saved in one transaction (NFR-08).
class MarkingService {
public:
    MarkingService(Database& db, ISubmissionRepository& submissions,
                   IAssignmentRepository& assignments, ICourseRepository& courses,
                   const IPenaltyPolicy& policy, AuditService& audit);

    // The submissions of one assignment, for a lecturer of its course.
    std::vector<Submission> listForMarking(int lecturerId, int assignmentId);

    // FR-12, FR-13. Raw mark 0 to max, feedback required; the penalty is applied.
    // Returns the final mark.
    double mark(int lecturerId, int submissionId, double rawMark, const std::string& feedback);

    // FR-18. Only the lecturer who marked it, only before moderation, with a reason.
    // Returns the new final mark.
    double amend(int lecturerId, int submissionId, double rawMark, const std::string& feedback,
               const std::string& reason);

    // The latest version of a student's work, so the lecturer can open the file before marking
    // (BR-05: the latest version is the one marked). Only for a lecturer of the course (FR-02).
    SubmissionVersion latestFile(int lecturerId, int submissionId);

private:
    Submission requireSubmission(int submissionId);
    Assignment requireAssignmentTaughtBy(int lecturerId, int assignmentId);
    void checkMark(double rawMark, const std::string& feedback, const Assignment& assignment);
    void save(Submission& submission, std::optional<double> oldFinal, int userId,
              const std::string& reason);

    Database& db_;
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    const IPenaltyPolicy& policy_;
    AuditService& audit_;
};

}  // namespace lms
