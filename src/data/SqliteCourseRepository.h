#pragma once
#include "data/Database.h"
#include "data/ICourseRepository.h"

namespace lms {

// Stores departments and courses, and the course_lecturer and enrolment junction tables.
class SqliteCourseRepository : public ICourseRepository {
public:
    explicit SqliteCourseRepository(Database& db);

    int saveDepartment(const Department& department) override;
    int saveCourse(const Course& course) override;
    std::optional<Department> findDepartment(int id) override;
    std::optional<Course> findCourse(int id) override;
    std::vector<Department> allDepartments() override;
    std::vector<Course> allCourses() override;

    void addLecturer(int courseId, int lecturerId) override;
    bool isLecturerOf(int courseId, int lecturerId) override;
    void addEnrolment(int courseId, int studentId) override;
    void removeEnrolment(int courseId, int studentId) override;
    bool isEnrolled(int courseId, int studentId) override;
    std::vector<Course> coursesForStudent(int studentId) override;
    std::vector<Course> coursesForLecturer(int lecturerId) override;

private:
    static Course readCourse(const Statement& row);

    Database& db_;
};

}  // namespace lms
