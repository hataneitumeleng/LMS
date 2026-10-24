#pragma once
#include "data/Database.h"
#include "data/ICourseRepository.h"
#include "data/IUserRepository.h"
#include "domain/UserFactory.h"
#include "services/IPasswordHasher.h"

#include <memory>
#include <string>
#include <vector>

namespace lms {

// Administrator functions (UC-02 to UC-05; FR-03 to FR-06).
// Only the administrator menu calls this service (FR-02).
class AdminService {
public:
    AdminService(Database& db, IUserRepository& users, ICourseRepository& courses,
                 const UserFactory& factory, const IPasswordHasher& hasher);

    // FR-03. HoDs are not created here: a lecturer becomes HoD through
    // createDepartment() or replaceHod(), because an HoD needs a department.
    int createUser(Role role, const std::string& username, const std::string& fullName,
                   const std::string& password);
    void updateUserName(int userId, const std::string& fullName);
    // Refuses an active HoD; swap them first with replaceHod() (Section 5.2).
    void deactivateUser(int userId);

    // FR-04. Creates the department and makes the lecturer its HoD in one transaction.
    int createDepartment(const std::string& name, int hodUserId);
    void renameDepartment(int departmentId, const std::string& name);
    // Deactivates the old HoD and makes the lecturer the new HoD, in one transaction.
    void replaceHod(int departmentId, int newHodUserId);

    int createCourse(const std::string& code, const std::string& title, int departmentId);
    void updateCourse(int courseId, const std::string& code, const std::string& title);

    void assignLecturer(int courseId, int lecturerId);  // FR-05
    void enrol(int courseId, int studentId);            // FR-06
    void unenrol(int courseId, int studentId);          // FR-06

    // Read-only lists for the administrator menu (the menus never use a repository).
    std::vector<std::unique_ptr<User>> listUsers();
    std::vector<Department> listDepartments();
    std::vector<Course> listCourses();

private:
    std::unique_ptr<User> requireUser(int userId, Role expectedRole);
    Course requireCourse(int courseId);
    void makeHod(const User& lecturer, int departmentId);

    Database& db_;
    IUserRepository& users_;
    ICourseRepository& courses_;
    const UserFactory& factory_;
    const IPasswordHasher& hasher_;
};

}  // namespace lms
