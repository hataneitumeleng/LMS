#include "domain/SubmissionStatus.h"

#include <iostream>

int main() {
    // Placeholder until the role menus are written (step 6).
    std::cout << "Moderated LMS v0.1.0\n";
    std::cout << "Statuses: ";
    for (auto s : {lms::SubmissionStatus::Submitted, lms::SubmissionStatus::Late,
                   lms::SubmissionStatus::Marked, lms::SubmissionStatus::Moderated,
                   lms::SubmissionStatus::Released}) {
        std::cout << lms::toString(s) << ' ';
    }
    std::cout << '\n';
    return 0;
}
