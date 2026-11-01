#include "services/AssignmentService.h"

#include "data/Transaction.h"
#include "domain/Errors.h"

#include <chrono>

namespace lms {

namespace {
constexpr int kMaxExtensionDays = 14;  // BR-03

bool isBlank(const std::string& value) {
    return value.find_first_not_of(" \t") == std::string::npos;
}
}  // namespace

AssignmentService::AssignmentService(Database& db, IAssignmentRepository& assignments,
                                     ICourseRepository& courses, IUserRepository& users,
                                     ISubmissionRepository& submissions,
                                     const IPenaltyPolicy& policy, const IClock& clock,
                                     const FileStore& store)
    : db_(db),
      assignments_(assignments),
      courses_(courses),
      users_(users),
      submissions_(submissions),
      policy_(policy),
      clock_(clock),
      store_(store) {}

int AssignmentService::createAssignment(int lecturerId, int courseId, const std::string& title,
                                        TimePoint deadline, int maxMark,
                                        const std::string& description) {
    if (!courses_.findCourse(courseId)) {
        throw ValidationError("No course with id " + std::to_string(courseId));
    }
    requireLecturerOf(lecturerId, courseId);
    if (isBlank(title)) throw ValidationError("Title must not be empty");
    if (maxMark < 1 || maxMark > 100) throw ValidationError("Max mark must be from 1 to 100");
    if (deadline <= clock_.now()) throw ValidationError("Deadline must be in the future");

    Assignment assignment;
    assignment.courseId = courseId;
    assignment.title = title;
    assignment.description = description;
    assignment.deadline = deadline;
    assignment.maxMark = maxMark;
    assignment.createdBy = lecturerId;
    return assignments_.save(assignment);
}

void AssignmentService::grantExtension(int lecturerId, int assignmentId, int studentId,
                                       TimePoint newDeadline, const std::string& reason) {
    Assignment assignment = requireAssignment(assignmentId);
    requireLecturerOf(lecturerId, assignment.courseId);
    if (!courses_.isEnrolled(assignment.courseId, studentId)) {
        throw ValidationError("Student " + std::to_string(studentId) +
                              " is not enrolled in this course");
    }
    // BR-03: later than the normal deadline, and at most 14 days after it
    if (newDeadline <= assignment.deadline) {
        throw ValidationError("The new deadline must be later than the normal deadline");
    }
    if (newDeadline > assignment.deadline + std::chrono::hours(24 * kMaxExtensionDays)) {
        throw ValidationError("An extension can be at most 14 days");
    }
    if (isBlank(reason)) throw ValidationError("A reason is required");

    // BR-03: an extension can only be granted until the work is marked
    std::optional<Submission> submission = submissions_.findCurrent(studentId, assignmentId);
    if (submission && !submission->isOpen()) {
        throw ValidationError("This student's work has already been marked");
    }

    Transaction tx(db_);
    assignments_.saveExtension(
        Extension{assignmentId, studentId, newDeadline, reason, lecturerId, clock_.now()});
    if (submission) {
        // BR-04: work out days late again against the new deadline
        // (a Late submission can go back to Submitted)
        const TimePoint submittedAt = submissions_.latestVersion(submission->id)->submittedAt;
        submission->setLateness(policy_.daysLate(submittedAt, newDeadline));
        submissions_.save(*submission);
    }
    tx.commit();
}

std::vector<Course> AssignmentService::coursesTaughtBy(int lecturerId) {
    return courses_.coursesForLecturer(lecturerId);
}

std::vector<Assignment> AssignmentService::assignmentsFor(int lecturerId, int courseId) {
    requireLecturerOf(lecturerId, courseId);
    return assignments_.forCourse(courseId);
}

void AssignmentService::attachBrief(int lecturerId, int assignmentId, const std::string& path) {
    Assignment assignment = requireAssignment(assignmentId);
    requireLecturerOf(lecturerId, assignment.courseId);
    store_.validate(path);  // the file must exist and not be empty

    const std::string oldBrief = assignment.briefPath;
    // Like a submission (Figure 6): copy the file, save the row, and delete the copy again
    // if saving fails, because a rollback can't undo a file copy.
    const std::string copy = store_.copyBrief(path, assignmentId);
    try {
        assignment.briefPath = copy;
        assignments_.save(assignment);
    } catch (...) {
        store_.remove(copy);
        throw;
    }
    if (!oldBrief.empty()) store_.remove(oldBrief);  // only after the new one is saved
}

void AssignmentService::requireLecturerOf(int lecturerId, int courseId) {
    std::unique_ptr<User> lecturer = users_.findById(lecturerId);
    // FR-02: only an active lecturer of this course may do this
    if (!lecturer || !lecturer->active() || !courses_.isLecturerOf(courseId, lecturerId)) {
        throw AuthError("You do not teach this course");
    }
}

Assignment AssignmentService::requireAssignment(int assignmentId) {
    std::optional<Assignment> assignment = assignments_.find(assignmentId);
    if (!assignment) {
        throw ValidationError("No assignment with id " + std::to_string(assignmentId));
    }
    return *assignment;
}

}  // namespace lms
