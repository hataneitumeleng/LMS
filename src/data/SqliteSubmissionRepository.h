#pragma once
#include "data/Database.h"
#include "data/ISubmissionRepository.h"

namespace lms {

// Stores the submission and submission_version tables.
class SqliteSubmissionRepository : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(Database& db);

    std::optional<Submission> findCurrent(int studentId, int assignmentId) override;
    std::optional<Submission> findById(int id) override;
    std::vector<Submission> findByAssignment(int assignmentId) override;
    int save(const Submission& submission) override;

    void addVersion(int submissionId, const SubmissionVersion& version) override;
    std::optional<SubmissionVersion> latestVersion(int submissionId) override;
    std::vector<SubmissionVersion> versions(int submissionId) override;
    std::vector<std::string> allFilePaths() override;
    int countNotModerated(int assignmentId) override;

private:
    static Submission readSubmission(const Statement& row);
    static SubmissionVersion readVersion(const Statement& row);

    Database& db_;
};

}  // namespace lms
