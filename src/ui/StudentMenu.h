#pragma once
#include "ui/Menu.h"

namespace lms {

// UC-09, UC-10, UC-11, UC-15.
class StudentMenu : public Menu {
public:
    StudentMenu(Console& io, Services& services, const User& user);

protected:
    std::string title() const override;
    std::vector<std::string> options() const override;
    void handle(int choice) override;

private:
    void viewAssignments();
    void submitWork();

    const User& user_;
};

}  // namespace lms
