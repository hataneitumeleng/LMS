#include "services/MarkingService.h"

#include "data/Transaction.h"
#include "domain/Errors.h"

namespace lms {

namespace {
bool isBlank(const std::string& value) {
    return value.find_first_not_of(" \t") == std::string::npos;
}
}  // namespace

MarkingService::MarkingService(Database& db, ISubmissionRepository& submissions,
                               IAssignmentRepository& assignments, ICourseRepository& courses,
                               const IPenaltyPolicy& policy, AuditService& audit)
    : db_(db),
      submissions_(submissions),
      assignments_(assignments),
      courses_(courses),
      policy_(policy),
      audit_(audit) {}

std::vector<Submission> MarkingService::listForMarking(int lecturerId, int assignmentId) {
    requireAssignmentTaughtBy(lecturerId, assignmentId);
    return submissions_.findByAssignment(assignmentId);
}

double MarkingService::mark(int lecturerId, int submissionId, double rawMark,
                          const std::string& feedback) {
    Submission submission = requireSubmission(submissionId);
    const Assignment assignment = requireAssignmentTaughtBy(lecturerId, submission.assignmentId);
    if (!submission.isOpen()) {
        throw ValidationError("This work is already marked. Use amend to change the mark");
    }
    checkMark(rawMark, feedback, assignment);

    // FR-13: the penalty is applied automatically from the stored days late
    const double finalMark = policy_.finalMark(rawMark, assignment.maxMark, submission.daysLate);
    submission.recordMark(rawMark, finalMark, feedback, lecturerId);
    save(submission, std::nullopt, lecturerId, "First mark");
    return finalMark;
}

double MarkingService::amend(int lecturerId, int submissionId, double rawMark,
                           const std::string& feedback, const std::string& reason) {
    Submission submission = requireSubmission(submissionId);
    const Assignment assignment = requireAssignmentTaughtBy(lecturerId, submission.assignmentId);
    if (submission.status != SubmissionStatus::Marked) {
        throw ValidationError(submission.isOpen()
                                  ? "This work is not marked yet"
                                  : "This mark has been moderated; only the HoD can change it");
    }
    // FR-18: a lecturer amends only their own mark
    if (submission.markedBy != lecturerId) {
        throw AuthError("Only the lecturer who marked this work can amend it");
    }
    if (isBlank(reason)) throw ValidationError("A reason is required to change a mark");
    checkMark(rawMark, feedback, assignment);

    const std::optional<double> oldFinal = submission.finalMark;
    const double finalMark = policy_.finalMark(rawMark, assignment.maxMark, submission.daysLate);
    submission.recordMark(rawMark, finalMark, feedback, lecturerId);
    save(submission, oldFinal, lecturerId, reason);
    return finalMark;
}

SubmissionVersion MarkingService::latestFile(int lecturerId, int submissionId) {
    const Submission submission = requireSubmission(submissionId);
    requireAssignmentTaughtBy(lecturerId, submission.assignmentId);
    std::optional<SubmissionVersion> version = submissions_.latestVersion(submissionId);
    if (!version) throw ValidationError("This submission has no file");
    return *version;
}

Submission MarkingService::requireSubmission(int submissionId) {
    std::optional<Submission> submission = submissions_.findById(submissionId);
    if (!submission) {
        throw ValidationError("No submission with id " + std::to_string(submissionId));
    }
    return *submission;
}

Assignment MarkingService::requireAssignmentTaughtBy(int lecturerId, int assignmentId) {
    std::optional<Assignment> assignment = assignments_.find(assignmentId);
    if (!assignment) {
        throw ValidationError("No assignment with id " + std::to_string(assignmentId));
    }
    // FR-02: only a lecturer of the course may mark its work
    if (!courses_.isLecturerOf(assignment->courseId, lecturerId)) {
        throw AuthError("You do not teach this course");
    }
    return *assignment;
}

void MarkingService::checkMark(double rawMark, const std::string& feedback,
                               const Assignment& assignment) {
    if (rawMark < 0 || rawMark > assignment.maxMark) {
        throw ValidationError("The mark must be from 0 to " + std::to_string(assignment.maxMark));
    }
    if (isBlank(feedback)) throw ValidationError("Feedback is required");  // A8
}

void MarkingService::save(Submission& submission, std::optional<double> oldFinal, int userId,
                          const std::string& reason) {
    // NFR-08: the mark and its audit entry are saved together or not at all
    Transaction tx(db_);
    submissions_.save(submission);
    audit_.record(userId, submission.id, oldFinal, *submission.finalMark, reason);
    tx.commit();
}

}  // namespace lms
