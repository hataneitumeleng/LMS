#pragma once
#include "ui/Menu.h"

#include <memory>

namespace lms {

// UC-01. The first screen: log in, then the menu for the user's role (FR-02).
class LoginMenu : public Menu {
public:
    LoginMenu(Console& io, Services& services);

protected:
    std::string title() const override { return "Moderated LMS"; }
    std::vector<std::string> options() const override { return {"Log in"}; }
    void handle(int choice) override;
    std::string exitLabel() const override { return "Exit"; }

private:
    // Polymorphism: returns the right Menu subclass for the user's role.
    std::unique_ptr<Menu> menuFor(const User& user);
};

}  // namespace lms
