#pragma once
#include "data/Database.h"
#include "data/IAssignmentRepository.h"
#include "data/ICourseRepository.h"
#include "data/ISubmissionRepository.h"
#include "services/FileStore.h"
#include "services/IClock.h"
#include "services/IPenaltyPolicy.h"

#include <string>
#include <vector>

namespace lms {

// One line of the student's assignment list (FR-09).
struct StudentAssignmentRow {
    std::string courseCode;
    int assignmentId = 0;
    std::string title;
    TimePoint effectiveDeadline;
    std::string status;    // what the student may see (BR-07)
    std::string feedback;  // only filled in once released (FR-16)
    bool hasBrief = false;  // the lecturer attached a brief file (FR-24)
};

// Student functions: list assignments, submit and resubmit (UC-08, UC-09; FR-09 to FR-11).
class SubmissionService {
public:
    SubmissionService(Database& db, ISubmissionRepository& submissions,
                      IAssignmentRepository& assignments, ICourseRepository& courses,
                      const IPenaltyPolicy& policy, const IClock& clock, const FileStore& store);

    // Figure 6. First submission: Submitted or Late (FR-10).
    // Before the effective deadline: a new version (FR-11, BR-05).
    // After the deadline or once marked: refused (BR-06).
    Submission submit(int studentId, int assignmentId, const std::string& path);

    std::vector<StudentAssignmentRow> listForStudent(int studentId);

    // Called at start-up: deletes stored files the database doesn't know about.
    // FR-24: the path of an assignment's brief file, for a student enrolled in its course.
    std::string briefFor(int studentId, int assignmentId);

    // Deletes files in the submissions folder that no submission or brief refers to.
    int removeOrphanFiles();

private:
    Submission firstSubmission(int studentId, const Assignment& assignment, TimePoint now,
                               TimePoint deadline, const std::string& path);
    Submission resubmission(Submission existing, TimePoint now, TimePoint deadline,
                            const std::string& path);
    static std::string statusForStudent(const std::optional<Submission>& submission,
                                        int maxMark);

    Database& db_;
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    const IPenaltyPolicy& policy_;
    const IClock& clock_;
    const FileStore& store_;
};

}  // namespace lms
