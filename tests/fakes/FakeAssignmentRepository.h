#pragma once
#include "data/IAssignmentRepository.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace lms::test {

// IAssignmentRepository kept in memory. Like the real one, a second extension for the same
// student replaces the first (primary key assignment + student).
class FakeAssignmentRepository : public IAssignmentRepository {
public:
    std::optional<Assignment> find(int id) override {
        const auto it = assignments_.find(id);
        if (it == assignments_.end()) return std::nullopt;
        return it->second;
    }
    std::vector<Assignment> forCourse(int courseId) override {
        std::vector<Assignment> result;
        for (const auto& [id, a] : assignments_) {
            if (a.courseId == courseId) result.push_back(a);
        }
        std::sort(result.begin(), result.end(),
                  [](const Assignment& x, const Assignment& y) { return x.deadline < y.deadline; });
        return result;
    }
    int save(const Assignment& assignment) override {
        if (failNextSave) {
            failNextSave = false;
            throw std::runtime_error("simulated database error");
        }
        Assignment a = assignment;
        if (a.id == 0) a.id = nextId_++;
        assignments_[a.id] = a;
        return a.id;
    }
    std::optional<Extension> findExtension(int assignmentId, int studentId) override {
        const auto it = extensions_.find({assignmentId, studentId});
        if (it == extensions_.end()) return std::nullopt;
        return it->second;
    }
    void saveExtension(const Extension& extension) override {
        extensions_[{extension.assignmentId, extension.studentId}] = extension;
    }
    std::vector<std::string> allBriefPaths() override {
        std::vector<std::string> paths;
        for (const auto& [id, a] : assignments_) {
            if (!a.briefPath.empty()) paths.push_back(a.briefPath);
        }
        return paths;
    }

    bool failNextSave = false;  // makes the next save() throw, like a full disk

private:
    std::map<int, Assignment> assignments_;
    std::map<std::pair<int, int>, Extension> extensions_;
    int nextId_ = 1;
};

}  // namespace lms::test
