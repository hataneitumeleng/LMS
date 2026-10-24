#pragma once
#include "ui/Menu.h"

namespace lms {

// UC-06, UC-07, UC-08, UC-15; FR-18.
class LecturerMenu : public Menu {
public:
    LecturerMenu(Console& io, Services& services, const User& user);

protected:
    std::string title() const override;
    std::vector<std::string> options() const override;
    void handle(int choice) override;

private:
    void viewCourses();
    void createAssignment();
    void grantExtension();
    void viewSubmissions();
    void markSubmission();
    void amendMark();

    const User& user_;
};

}  // namespace lms
