#pragma once
#include "ui/Menu.h"

namespace lms {

// UC-02 to UC-05, UC-14, UC-15, UC-16. The administrator has more than 9 actions,
// so users and courses each get their own sub-menu (NFR-06).
class AdminMenu : public Menu {
public:
    AdminMenu(Console& io, Services& services, const User& user);

protected:
    std::string title() const override;
    std::vector<std::string> options() const override;
    void handle(int choice) override;

private:
    void backUp();

    const User& user_;
};

// Sub-menu: user accounts (UC-02, FR-03).
class AdminUsersMenu : public Menu {
public:
    AdminUsersMenu(Console& io, Services& services);

protected:
    std::string title() const override { return "Users"; }
    std::vector<std::string> options() const override;
    void handle(int choice) override;
    std::string exitLabel() const override { return "Back"; }

private:
    void listUsers();
    void createUser();
    void renameUser();
    void deactivateUser();
};

// Sub-menu: departments, courses and enrolment (UC-03 to UC-05; FR-04 to FR-06).
class AdminCoursesMenu : public Menu {
public:
    AdminCoursesMenu(Console& io, Services& services);

protected:
    std::string title() const override { return "Departments, courses and enrolment"; }
    std::vector<std::string> options() const override;
    void handle(int choice) override;
    std::string exitLabel() const override { return "Back"; }

private:
    void listAll();
    void createDepartment();
    void renameDepartment();
    void replaceHod();
    void createCourse();
    void updateCourse();
    void assignLecturer();
    void enrol();
    void unenrol();
};

}  // namespace lms
