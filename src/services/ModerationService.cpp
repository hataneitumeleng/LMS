#include "services/ModerationService.h"

#include "data/Transaction.h"
#include "domain/Errors.h"

namespace lms {

ModerationService::ModerationService(Database& db, ISubmissionRepository& submissions,
                                     IAssignmentRepository& assignments,
                                     ICourseRepository& courses, IUserRepository& users,
                                     AuditService& audit, const IClock& clock)
    : db_(db),
      submissions_(submissions),
      assignments_(assignments),
      courses_(courses),
      users_(users),
      audit_(audit),
      clock_(clock) {}

std::vector<Submission> ModerationService::listForModeration(int hodId, int assignmentId) {
    requireAssignmentInDepartment(hodId, assignmentId);
    std::vector<Submission> marked;
    for (const Submission& s : submissions_.findByAssignment(assignmentId)) {
        if (!s.isOpen()) marked.push_back(s);
    }
    return marked;
}

int ModerationService::approveAll(int hodId, int assignmentId) {
    requireAssignmentInDepartment(hodId, assignmentId);
    int approved = 0;
    Transaction tx(db_);
    for (Submission s : submissions_.findByAssignment(assignmentId)) {
        if (s.status != SubmissionStatus::Marked) continue;
        s.moderate(*s.finalMark, hodId, clock_.now());  // same mark, so no audit entry
        submissions_.save(s);
        ++approved;
    }
    tx.commit();
    return approved;
}

void ModerationService::adjust(int hodId, int submissionId, double finalMark,
                               const std::string& reason) {
    std::optional<Submission> submission = submissions_.findById(submissionId);
    if (!submission) {
        throw ValidationError("No submission with id " + std::to_string(submissionId));
    }
    const Assignment assignment = requireAssignmentInDepartment(hodId, submission->assignmentId);
    if (finalMark < 0 || finalMark > assignment.maxMark) {
        throw ValidationError("The mark must be from 0 to " + std::to_string(assignment.maxMark));
    }
    // BR-08: an adjustment needs a written reason
    if (reason.find_first_not_of(" \t") == std::string::npos) {
        throw ValidationError("A reason is required to adjust a mark");
    }

    const std::optional<double> oldFinal = submission->finalMark;
    submission->moderate(finalMark, hodId, clock_.now());

    // NFR-08: the adjustment and its audit entry are saved together or not at all
    Transaction tx(db_);
    submissions_.save(*submission);
    audit_.record(hodId, submissionId, oldFinal, finalMark, reason);
    tx.commit();
}

int ModerationService::release(int hodId, int assignmentId) {
    requireAssignmentInDepartment(hodId, assignmentId);
    // The check and the release are in one transaction, so nothing can change in between.
    Transaction tx(db_);
    const int notModerated = submissions_.countNotModerated(assignmentId);
    if (notModerated > 0) {
        throw ValidationError("Cannot release: " + std::to_string(notModerated) +
                              " submission(s) are not moderated yet");
    }
    int released = 0;
    for (Submission s : submissions_.findByAssignment(assignmentId)) {
        if (s.status != SubmissionStatus::Moderated) continue;
        s.release();
        submissions_.save(s);
        ++released;
    }
    if (released == 0) throw ValidationError("There is nothing to release");
    tx.commit();
    return released;
}

Assignment ModerationService::requireAssignmentInDepartment(int hodId, int assignmentId) {
    std::optional<Assignment> assignment = assignments_.find(assignmentId);
    if (!assignment) {
        throw ValidationError("No assignment with id " + std::to_string(assignmentId));
    }
    std::unique_ptr<User> hod = users_.findById(hodId);
    const std::optional<Course> course = courses_.findCourse(assignment->courseId);
    // FR-02, BR-08: only the active HoD of the course's own department
    if (!hod || !hod->active() || hod->role() != Role::HoD ||
        hod->departmentId() != course->departmentId) {
        throw AuthError("Only the HoD of this course's department can do this");
    }
    return *assignment;
}

}  // namespace lms
