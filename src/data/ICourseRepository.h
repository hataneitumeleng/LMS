#pragma once
#include "domain/Course.h"
#include "domain/Department.h"

#include <optional>
#include <vector>

namespace lms {

// Departments, courses, lecturers of a course and enrolments (FR-04, FR-05, FR-06).
class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    // Insert when id is 0, otherwise update. Return the id.
    virtual int saveDepartment(const Department& department) = 0;
    virtual int saveCourse(const Course& course) = 0;
    virtual std::optional<Department> findDepartment(int id) = 0;
    virtual std::optional<Course> findCourse(int id) = 0;
    virtual std::vector<Course> allCourses() = 0;

    virtual void addLecturer(int courseId, int lecturerId) = 0;
    virtual bool isLecturerOf(int courseId, int lecturerId) = 0;
    virtual void addEnrolment(int courseId, int studentId) = 0;
    virtual void removeEnrolment(int courseId, int studentId) = 0;
    virtual std::vector<Course> coursesForStudent(int studentId) = 0;
};

}  // namespace lms
