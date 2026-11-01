#pragma once
#include "data/ICourseRepository.h"

#include <map>
#include <set>
#include <utility>
#include <vector>

namespace lms::test {

// ICourseRepository kept in memory: departments, courses, who teaches and who is enrolled.
class FakeCourseRepository : public ICourseRepository {
public:
    int saveDepartment(const Department& department) override {
        Department d = department;
        if (d.id == 0) d.id = nextDepartment_++;
        departments_[d.id] = d;
        return d.id;
    }
    int saveCourse(const Course& course) override {
        Course c = course;
        if (c.id == 0) c.id = nextCourse_++;
        courses_[c.id] = c;
        return c.id;
    }
    std::optional<Department> findDepartment(int id) override {
        const auto it = departments_.find(id);
        if (it == departments_.end()) return std::nullopt;
        return it->second;
    }
    std::optional<Course> findCourse(int id) override {
        const auto it = courses_.find(id);
        if (it == courses_.end()) return std::nullopt;
        return it->second;
    }
    std::vector<Department> allDepartments() override {
        std::vector<Department> all;
        for (const auto& [id, d] : departments_) all.push_back(d);
        return all;
    }
    std::vector<Course> allCourses() override {
        std::vector<Course> all;
        for (const auto& [id, c] : courses_) all.push_back(c);
        return all;
    }
    void addLecturer(int courseId, int lecturerId) override {
        lecturers_.insert({courseId, lecturerId});
    }
    bool isLecturerOf(int courseId, int lecturerId) override {
        return lecturers_.count({courseId, lecturerId}) > 0;
    }
    void addEnrolment(int courseId, int studentId) override {
        enrolments_.insert({courseId, studentId});
    }
    void removeEnrolment(int courseId, int studentId) override {
        enrolments_.erase({courseId, studentId});
    }
    bool isEnrolled(int courseId, int studentId) override {
        return enrolments_.count({courseId, studentId}) > 0;
    }
    std::vector<Course> coursesForStudent(int studentId) override {
        std::vector<Course> result;
        for (const auto& [courseId, student] : enrolments_) {
            if (student == studentId) result.push_back(courses_.at(courseId));
        }
        return result;
    }
    std::vector<Course> coursesForLecturer(int lecturerId) override {
        std::vector<Course> result;
        for (const auto& [courseId, lecturer] : lecturers_) {
            if (lecturer == lecturerId) result.push_back(courses_.at(courseId));
        }
        return result;
    }

private:
    std::map<int, Department> departments_;
    std::map<int, Course> courses_;
    std::set<std::pair<int, int>> lecturers_;   // (course, lecturer)
    std::set<std::pair<int, int>> enrolments_;  // (course, student)
    int nextDepartment_ = 1;
    int nextCourse_ = 1;
};

}  // namespace lms::test
