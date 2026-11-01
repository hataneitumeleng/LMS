// Service tests for AdminService: users, departments and HoDs, courses, teaching and
// enrolment (UC-02 to UC-05; FR-03 to FR-06). Fixture: see ServiceTest.h.
#include "../fakes/ServiceTest.h"
#include "domain/Errors.h"

namespace lms::test {
namespace {

class AdminServiceTest : public ServiceTest {
protected:
    std::unique_ptr<User> user(int id) { return users.findById(id); }
};

// ---- Users (FR-03) ----

TEST_F(AdminServiceTest, CreateUserStoresAHashNotThePassword) {  // NFR-05
    const int id = adminService.createUser(Role::Student, "lindi", "Lindi Dube", "Secret123");
    const auto saved = user(id);
    ASSERT_TRUE(saved);
    EXPECT_EQ(saved->role(), Role::Student);
    EXPECT_EQ(saved->fullName(), "Lindi Dube");
    EXPECT_NE(saved->passwordHash(), "Secret123");
    EXPECT_TRUE(hasher.verify("Secret123", saved->passwordHash()));
}

TEST_F(AdminServiceTest, NewUserCanLogIn) {
    adminService.createUser(Role::Lecturer, "kabelo", "Kabelo M", "Secret123");
    EXPECT_EQ(auth.login("kabelo", "Secret123")->role(), Role::Lecturer);
}

TEST_F(AdminServiceTest, UsernameMustBeUnique) {
    EXPECT_THROW(adminService.createUser(Role::Student, "itu", "Another Itu", "Secret123"),
                 ValidationError);
}

TEST_F(AdminServiceTest, ShortPasswordIsRefused) {
    EXPECT_THROW(adminService.createUser(Role::Student, "lindi", "Lindi", "Short12"),
                 ValidationError);
}

TEST_F(AdminServiceTest, BlankUsernameOrNameIsRefused) {
    EXPECT_THROW(adminService.createUser(Role::Student, " ", "Lindi", "Secret123"),
                 ValidationError);
    EXPECT_THROW(adminService.createUser(Role::Student, "lindi", "", "Secret123"),
                 ValidationError);
}

TEST_F(AdminServiceTest, HodIsNotCreatedDirectly) {  // an HoD needs a department
    EXPECT_THROW(adminService.createUser(Role::HoD, "boss", "Boss", "Secret123"),
                 ValidationError);
}

TEST_F(AdminServiceTest, RenameKeepsRoleAndPassword) {
    adminService.updateUserName(itu, "Itumeleng Hatane");
    EXPECT_EQ(user(itu)->fullName(), "Itumeleng Hatane");
    EXPECT_EQ(user(itu)->role(), Role::Student);
    EXPECT_NO_THROW(auth.login("itu", "Password1"));
}

TEST_F(AdminServiceTest, DeactivatedUserCannotLogIn) {
    adminService.deactivateUser(neo);
    EXPECT_FALSE(user(neo)->active());
    EXPECT_THROW(auth.login("neo", "Password1"), AuthError);
}

TEST_F(AdminServiceTest, ActiveHodCannotBeDeactivated) {  // replace them first
    EXPECT_THROW(adminService.deactivateUser(mpho), ValidationError);
    EXPECT_TRUE(user(mpho)->active());
}

// ---- Departments and HoDs (FR-04) ----

TEST_F(AdminServiceTest, CreateDepartmentMakesTheLecturerItsHod) {
    const int arts = adminService.createDepartment("Arts", lerato);
    const auto hod = user(lerato);
    EXPECT_EQ(hod->role(), Role::HoD);
    EXPECT_EQ(hod->departmentId(), arts);
    EXPECT_EQ(hod->username(), "lerato");                 // the same account
    EXPECT_NO_THROW(auth.login("lerato", "Password1"));  // with the same password
}

TEST_F(AdminServiceTest, OnlyALecturerCanBecomeHod) {
    EXPECT_THROW(adminService.createDepartment("Arts", itu), ValidationError);
    EXPECT_THROW(adminService.createDepartment("Arts", 99), ValidationError);
}

TEST_F(AdminServiceTest, DepartmentNameIsRequired) {
    EXPECT_THROW(adminService.createDepartment("  ", lerato), ValidationError);
}

TEST_F(AdminServiceTest, ReplaceHodSwapsTheHods) {
    adminService.replaceHod(science, lerato);
    EXPECT_FALSE(user(mpho)->active());  // kept for the audit trail, but deactivated
    EXPECT_EQ(user(lerato)->role(), Role::HoD);
    EXPECT_EQ(users.findActiveHod(science)->id(), lerato);
}

TEST_F(AdminServiceTest, ReplaceHodOfUnknownDepartmentIsRefused) {
    EXPECT_THROW(adminService.replaceHod(99, lerato), ValidationError);
}

TEST_F(AdminServiceTest, RenameDepartment) {
    adminService.renameDepartment(science, "Computing");
    EXPECT_EQ(courses.findDepartment(science)->name, "Computing");
    EXPECT_THROW(adminService.renameDepartment(99, "X"), ValidationError);
}

// ---- Courses, teaching and enrolment (FR-05, FR-06) ----

TEST_F(AdminServiceTest, CreateAndUpdateCourse) {
    const int id = adminService.createCourse("CS102", "Programming 2", science);
    adminService.updateCourse(id, "CS102", "Data Structures");
    EXPECT_EQ(courses.findCourse(id)->title, "Data Structures");
    EXPECT_EQ(courses.findCourse(id)->departmentId, science);
}

TEST_F(AdminServiceTest, CourseNeedsAKnownDepartmentAndText) {
    EXPECT_THROW(adminService.createCourse("CS102", "Programming 2", 99), ValidationError);
    EXPECT_THROW(adminService.createCourse("", "Programming 2", science), ValidationError);
    EXPECT_THROW(adminService.createCourse("CS102", " ", science), ValidationError);
}

TEST_F(AdminServiceTest, AssignLecturerLetsThemTeach) {
    adminService.assignLecturer(cs101, lerato);
    EXPECT_TRUE(courses.isLecturerOf(cs101, lerato));
    EXPECT_NO_THROW(assignmentService.assignmentsFor(lerato, cs101));
}

TEST_F(AdminServiceTest, OnlyLecturersTeachAndOnlyStudentsEnrol) {
    EXPECT_THROW(adminService.assignLecturer(cs101, itu), ValidationError);
    EXPECT_THROW(adminService.enrol(cs101, thabo), ValidationError);
    EXPECT_THROW(adminService.enrol(99, sipho), ValidationError);
}

TEST_F(AdminServiceTest, EnrolAndUnenrol) {
    adminService.enrol(cs101, sipho);
    EXPECT_TRUE(courses.isEnrolled(cs101, sipho));
    EXPECT_EQ(submissionService.listForStudent(sipho).size(), 1u);  // now sees Project 1
    adminService.unenrol(cs101, sipho);
    EXPECT_FALSE(courses.isEnrolled(cs101, sipho));
}

TEST_F(AdminServiceTest, ListsForTheMenus) {
    EXPECT_EQ(adminService.listUsers().size(), 7u);
    EXPECT_EQ(adminService.listDepartments().size(), 1u);
    EXPECT_EQ(adminService.listCourses().size(), 1u);
}

}  // namespace
}  // namespace lms::test
