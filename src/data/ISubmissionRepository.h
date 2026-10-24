#pragma once
#include "domain/Submission.h"
#include "domain/SubmissionVersion.h"

#include <optional>
#include <string>
#include <vector>

namespace lms {

// Submissions and their file versions (FR-10, FR-11).
class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    // The one submission a student has for an assignment, if any.
    virtual std::optional<Submission> findCurrent(int studentId, int assignmentId) = 0;
    virtual std::optional<Submission> findById(int id) = 0;
    virtual std::vector<Submission> findByAssignment(int assignmentId) = 0;
    // Insert when id is 0, otherwise update. Return the id.
    virtual int save(const Submission& submission) = 0;

    virtual void addVersion(int submissionId, const SubmissionVersion& version) = 0;
    // The current version is the one with the highest number (BR-05).
    virtual std::optional<SubmissionVersion> latestVersion(int submissionId) = 0;
    virtual std::vector<SubmissionVersion> versions(int submissionId) = 0;
    // Every stored file path, so FileStore can delete files left by a crash.
    virtual std::vector<std::string> allFilePaths() = 0;
    // Submissions of an assignment that are still Submitted, Late or Marked (FR-15).
    virtual int countNotModerated(int assignmentId) = 0;
};

}  // namespace lms
