#pragma once
#include "ui/Menu.h"

namespace lms {

// UC-12, UC-13, UC-14, UC-15 (Figure 7).
class HodMenu : public Menu {
public:
    HodMenu(Console& io, Services& services, const User& user);

protected:
    std::string title() const override;
    std::vector<std::string> options() const override;
    void handle(int choice) override;

private:
    void viewAssignments();
    void viewMarkedWork();
    void approveAll();
    void adjustMark();
    void releaseMarks();

    const User& user_;
};

}  // namespace lms
