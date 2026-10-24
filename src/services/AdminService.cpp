#include "services/AdminService.h"

#include "data/Transaction.h"
#include "domain/Errors.h"
#include "services/AuthService.h"

namespace lms {

namespace {
void requireText(const std::string& value, const std::string& field) {
    if (value.find_first_not_of(" \t") == std::string::npos) {
        throw ValidationError(field + " must not be empty");
    }
}
}  // namespace

AdminService::AdminService(Database& db, IUserRepository& users, ICourseRepository& courses,
                           const UserFactory& factory, const IPasswordHasher& hasher)
    : db_(db), users_(users), courses_(courses), factory_(factory), hasher_(hasher) {}

int AdminService::createUser(Role role, const std::string& username,
                             const std::string& fullName, const std::string& password) {
    if (role == Role::HoD) {
        throw ValidationError("Create the lecturer first, then make them HoD of a department");
    }
    requireText(username, "Username");
    requireText(fullName, "Full name");
    AuthService::checkPasswordRules(password);
    if (users_.findByUsername(username)) {
        throw ValidationError("Username '" + username + "' is already taken");
    }
    std::unique_ptr<User> user = factory_.create(role, username, fullName, std::nullopt);
    user->setPasswordHash(hasher_.hash(password));
    return users_.save(*user);
}

void AdminService::updateUserName(int userId, const std::string& fullName) {
    requireText(fullName, "Full name");
    std::unique_ptr<User> user = users_.findById(userId);
    if (!user) throw ValidationError("No user with id " + std::to_string(userId));
    std::unique_ptr<User> renamed =
        factory_.create(user->role(), user->username(), fullName, user->departmentId());
    renamed->setId(user->id());
    renamed->setPasswordHash(user->passwordHash());
    renamed->setActive(user->active());
    renamed->setLoginState(user->failedAttempts(), user->lockedUntil());
    users_.save(*renamed);
}

void AdminService::deactivateUser(int userId) {
    std::unique_ptr<User> user = users_.findById(userId);
    if (!user) throw ValidationError("No user with id " + std::to_string(userId));
    if (user->role() == Role::HoD && user->active()) {
        throw ValidationError("This user is an active HoD. Replace the HoD first");
    }
    user->setActive(false);
    users_.save(*user);
}

int AdminService::createDepartment(const std::string& name, int hodUserId) {
    requireText(name, "Department name");
    std::unique_ptr<User> lecturer = requireUser(hodUserId, Role::Lecturer);

    Transaction tx(db_);
    const int departmentId = courses_.saveDepartment(Department{0, name});
    makeHod(*lecturer, departmentId);
    tx.commit();
    return departmentId;
}

void AdminService::renameDepartment(int departmentId, const std::string& name) {
    requireText(name, "Department name");
    if (!courses_.findDepartment(departmentId)) {
        throw ValidationError("No department with id " + std::to_string(departmentId));
    }
    courses_.saveDepartment(Department{departmentId, name});
}

void AdminService::replaceHod(int departmentId, int newHodUserId) {
    std::unique_ptr<User> lecturer = requireUser(newHodUserId, Role::Lecturer);
    std::unique_ptr<User> oldHod = users_.findActiveHod(departmentId);
    if (!oldHod) {
        throw ValidationError("No department with id " + std::to_string(departmentId));
    }

    Transaction tx(db_);
    // The old HoD is deactivated first, because the database allows only one
    // active HoD per department. The row stays so their audit entries keep a valid user.
    oldHod->setActive(false);
    users_.save(*oldHod);
    makeHod(*lecturer, departmentId);
    tx.commit();
}

int AdminService::createCourse(const std::string& code, const std::string& title,
                               int departmentId) {
    requireText(code, "Course code");
    requireText(title, "Course title");
    if (!courses_.findDepartment(departmentId)) {
        throw ValidationError("No department with id " + std::to_string(departmentId));
    }
    return courses_.saveCourse(Course{0, code, title, departmentId});
}

void AdminService::updateCourse(int courseId, const std::string& code, const std::string& title) {
    requireText(code, "Course code");
    requireText(title, "Course title");
    Course course = requireCourse(courseId);
    course.code = code;
    course.title = title;
    courses_.saveCourse(course);
}

void AdminService::assignLecturer(int courseId, int lecturerId) {
    requireCourse(courseId);
    requireUser(lecturerId, Role::Lecturer);
    courses_.addLecturer(courseId, lecturerId);
}

void AdminService::enrol(int courseId, int studentId) {
    requireCourse(courseId);
    requireUser(studentId, Role::Student);
    courses_.addEnrolment(courseId, studentId);
}

void AdminService::unenrol(int courseId, int studentId) {
    requireCourse(courseId);
    courses_.removeEnrolment(courseId, studentId);
}

std::unique_ptr<User> AdminService::requireUser(int userId, Role expectedRole) {
    std::unique_ptr<User> user = users_.findById(userId);
    if (!user || !user->active()) {
        throw ValidationError("No active user with id " + std::to_string(userId));
    }
    if (user->role() != expectedRole) {
        throw ValidationError(user->username() + " is not a " + toString(expectedRole));
    }
    return user;
}

Course AdminService::requireCourse(int courseId) {
    std::optional<Course> course = courses_.findCourse(courseId);
    if (!course) throw ValidationError("No course with id " + std::to_string(courseId));
    return *course;
}

void AdminService::makeHod(const User& lecturer, int departmentId) {
    // The same account (id, username, password) becomes a HeadOfDepartment object.
    std::unique_ptr<User> hod =
        factory_.create(Role::HoD, lecturer.username(), lecturer.fullName(), departmentId);
    hod->setId(lecturer.id());
    hod->setPasswordHash(lecturer.passwordHash());
    hod->setLoginState(lecturer.failedAttempts(), lecturer.lockedUntil());
    users_.save(*hod);
}

}  // namespace lms
