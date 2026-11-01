#pragma once
#include "data/ISubmissionRepository.h"

#include <map>
#include <stdexcept>
#include <vector>

namespace lms::test {

// ISubmissionRepository kept in memory, with a switch that makes addVersion() fail so the
// tests can check the file clean-up of Figure 6.
class FakeSubmissionRepository : public ISubmissionRepository {
public:
    std::optional<Submission> findCurrent(int studentId, int assignmentId) override {
        for (const auto& [id, s] : submissions_) {
            if (s.studentId == studentId && s.assignmentId == assignmentId) return s;
        }
        return std::nullopt;
    }
    std::optional<Submission> findById(int id) override {
        const auto it = submissions_.find(id);
        if (it == submissions_.end()) return std::nullopt;
        return it->second;
    }
    std::vector<Submission> findByAssignment(int assignmentId) override {
        std::vector<Submission> result;
        for (const auto& [id, s] : submissions_) {
            if (s.assignmentId == assignmentId) result.push_back(s);
        }
        return result;
    }
    int save(const Submission& submission) override {
        Submission s = submission;
        if (s.id == 0) s.id = nextId_++;
        submissions_[s.id] = s;
        return s.id;
    }
    void addVersion(int submissionId, const SubmissionVersion& version) override {
        if (failNextAddVersion) {
            failNextAddVersion = false;
            throw std::runtime_error("simulated database error");
        }
        versions_[submissionId].push_back(version);
    }
    std::optional<SubmissionVersion> latestVersion(int submissionId) override {
        const auto it = versions_.find(submissionId);
        if (it == versions_.end() || it->second.empty()) return std::nullopt;
        return it->second.back();
    }
    std::vector<SubmissionVersion> versions(int submissionId) override {
        return versions_[submissionId];
    }
    std::vector<std::string> allFilePaths() override {
        std::vector<std::string> paths;
        for (const auto& [id, list] : versions_) {
            for (const auto& v : list) paths.push_back(v.filePath);
        }
        return paths;
    }
    int countNotModerated(int assignmentId) override {
        int count = 0;
        for (const auto& [id, s] : submissions_) {
            if (s.assignmentId == assignmentId &&
                (s.isOpen() || s.status == SubmissionStatus::Marked)) {
                ++count;
            }
        }
        return count;
    }

    bool failNextAddVersion = false;  // makes the next addVersion() throw

private:
    std::map<int, Submission> submissions_;
    std::map<int, std::vector<SubmissionVersion>> versions_;
    int nextId_ = 1;
};

}  // namespace lms::test
