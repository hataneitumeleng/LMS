// GUI acceptance checks (Phase 5, step 5): the real Qt windows, driven from code.
//
// Each check GT-xx fills in fields and presses buttons exactly as a user would, then
// compares what the window shows or says with the expected result, prints PASS or FAIL
// with the actual message, and saves screenshots for the report.
//
// Usage: lms_gui_acceptance <folder>
//   The folder is emptied first; gui.db, the copied files and shots/*.png go into it.
//   Message boxes are replaced by MessageHooks (GuiCommon.h), so nothing waits for a click.
//   Exit code = number of failed checks (0 = all passed).
#include "app/System.h"
#include "gui/AdminWindow.h"
#include "gui/GuiApp.h"
#include "gui/GuiCommon.h"
#include "gui/HodWindow.h"
#include "gui/LecturerWindow.h"
#include "gui/LoginDialog.h"
#include "gui/StudentWindow.h"
#include "gui/ChangePasswordDialog.h"

#include <QApplication>
#include "domain/Errors.h"
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QPixmap>
#include <QDir>
#include <QFile>
#include <QDialogButtonBox>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace lms;
static QStringList msgs;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::cout << "    check failed (line " << __LINE__ << "): " #c "\n"; ++caseFails; } } while (0)
static int caseFails = 0;
static std::string current;
static QString outDir;
void begin(const std::string& id) { current = id; caseFails = 0; }
void end(const std::string& what) {
    std::cout << current << (caseFails ? " FAIL  " : " PASS  ") << what << "\n";
    if (caseFails) ++fails;
}
void note(const QString& s) { std::cout << "    actual: " << s.toStdString() << "\n"; }

template <class T> T* get(QWidget* w, const char* name) {
    T* x = w->findChild<T*>(name);
    if (!x) { std::cout << "missing widget " << name << "\n"; std::exit(2); }
    return x;
}
void set(QWidget* w, const char* n, const QString& t) { get<QLineEdit>(w, n)->setText(t); }
void click(QWidget* w, const char* n) { get<QPushButton>(w, n)->click(); }
QString last() { return msgs.isEmpty() ? "" : msgs.last(); }
void pick(QWidget* w, const char* n, const QString& textPart) {
    auto* c = get<QComboBox>(w, n);
    for (int i = 0; i < c->count(); ++i) if (c->itemText(i).contains(textPart)) { c->setCurrentIndex(i); return; }
    std::cout << "no combo item " << textPart.toStdString() << " in " << n << "\n"; ++fails;
}
void selectRowWith(QWidget* w, const char* n, int col, const QString& text) {
    auto* t = get<QTableWidget>(w, n);
    for (int r = 0; r < t->rowCount(); ++r) if (t->item(r, col)->text() == text) { t->selectRow(r); return; }
    std::cout << "no row " << text.toStdString() << " in " << n << "\n"; ++fails;
}
void tab(QWidget* w, int i) { w->findChild<QTabWidget*>()->setCurrentIndex(i); }
void shot(QWidget* w, const char* file) { w->resize(1000, 660); w->grab().save(outDir + "/" + file); }

std::unique_ptr<User> login(Services& s, const QString& u, const QString& p, QString* err = nullptr) {
    LoginDialog d(s.auth);
    set(&d, "username", u); set(&d, "password", p); click(&d, "login");
    if (err) *err = get<QLabel>(&d, "error")->text();
    if (d.result() != QDialog::Accepted) return nullptr;
    return d.takeUser();
}


int main(int argc, char** argv) {
    QApplication app(argc, argv);
    const std::filesystem::path folder = argc > 1 ? argv[1] : "gui-acceptance";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder / "shots");
    std::filesystem::current_path(folder);  // gui.db and the files go here
    outDir = "shots";
    messageHooks().info = [](QWidget*, const QString& t) { msgs << "INFO: " + t; };
    messageHooks().error = [](QWidget*, const QString& t) { msgs << "ERROR: " + t; };
    messageHooks().confirm = [](QWidget*, const QString& t) { msgs << "CONFIRM: " + t; return true; };
    messageHooks().openFile = [](const QString& p) { msgs << "OPEN: " + p; return true; };
    messageHooks().chooseFile = [](QWidget*, const QString&) { return QDir::currentPath() + "/brief.txt"; };
    std::ofstream("brief.txt") << "Brief v1\n";
    std::ofstream("essay.txt") << "My essay\n";

    System system("gui.db", "subs");
    Services& s = system.services();
    QString err;

    begin("GT-01");
    {
        FirstAdminDialog d(s.admin);
        set(&d, "username", "admin"); set(&d, "fullName", "System Admin");
        set(&d, "password", "Password1"); set(&d, "repeat", "Password2"); click(&d, "create");
        note(get<QLabel>(&d, "error")->text());
        CHECK(get<QLabel>(&d, "error")->text().contains("different"));
        set(&d, "repeat", "Password1"); click(&d, "create");
        CHECK(d.result() == QDialog::Accepted);
    }
    end("First start: create the administrator; different passwords are refused");

    begin("GT-02");
    {
        CHECK(!login(s, "admin", "wrongpass", &err)); note(err); CHECK(err.contains("Invalid"));
        LoginDialog l(s.auth); set(&l, "username", "admin"); set(&l, "password", "wrongpass");
        click(&l, "login"); l.resize(380, 200); l.grab().save(outDir + "/GT-02_login_refused.png");
        auto win = GuiApp::windowFor(s, login(s, "admin", "Password1"));
        CHECK(dynamic_cast<AdminWindow*>(win.get()));
    }
    end("Log in: wrong password refused, administrator gets the administrator window");

    begin("GT-03");
    {
        auto win = GuiApp::windowFor(s, login(s, "admin", "Password1"));
        QWidget* w = win.get();
        struct U { const char* role; const char* user; const char* name; };
        for (U u : {U{"Lecturer","mpho","Mpho Moloi"}, U{"Lecturer","thabo","Thabo Mokoena"},
                    U{"Student","itu","Itumeleng Hatane"}, U{"Student","neo","Neo Khumalo"}}) {
            pick(w, "role", u.role); set(w, "username", u.user); set(w, "fullName", u.name);
            set(w, "password", "Password1"); click(w, "createUser");
        }
        set(w, "username", "bad"); set(w, "fullName", "Bad"); set(w, "password", "short"); click(w, "createUser");
        note(last()); CHECK(last().startsWith("ERROR") && last().contains("8 characters"));
        CHECK(get<QTableWidget>(w, "users")->rowCount() == 5);
        shot(w, "GT-03_admin_users.png");
        end("Administrator creates users; a 5-letter password is refused");

        begin("GT-04");
        tab(w, 1);
        set(w, "departmentName", "Science"); pick(w, "hod", "Mpho"); click(w, "createDepartment");
        set(w, "courseCode", "CS101"); set(w, "courseTitle", "Programming 1"); pick(w, "courseDepartment", "Science");
        click(w, "createCourse"); note(last());
        CHECK(get<QTableWidget>(w, "departments")->item(0, 2)->text() == "Mpho Moloi");
        shot(w, "GT-04_admin_departments.png");
        end("Administrator creates a department with its HoD and a course");

        begin("GT-05");
        tab(w, 2);
        pick(w, "teachingCourse", "CS101"); pick(w, "lecturer", "Thabo"); click(w, "assignLecturer"); note(last());
        CHECK(last().startsWith("INFO"));
        pick(w, "student", "Itumeleng"); click(w, "enrol"); note(last());
        pick(w, "student", "Neo"); click(w, "enrol"); CHECK(last().startsWith("INFO"));
        shot(w, "GT-05_admin_teaching.png");
        end("Administrator assigns the lecturer and enrols two students");
    }

    begin("GT-06");
    {
        auto win = GuiApp::windowFor(s, login(s, "thabo", "Password1"));
        QWidget* w = win.get();
        CHECK(dynamic_cast<LecturerWindow*>(w));
        set(w, "title", ""); click(w, "createAssignment"); note(last());
        CHECK(last().contains("Title must not be empty"));
        set(w, "title", "Project 1");
        set(w, "brief", "\"" + QDir::currentPath() + "/brief.txt\"");
        get<QDateTimeEdit>(w, "deadline")->setDateTime(QDateTime::currentDateTime().addDays(5));
        click(w, "createAssignment"); note(last());
        CHECK(get<QTableWidget>(w, "assignments")->rowCount() == 1);
        CHECK(get<QTableWidget>(w, "assignments")->item(0, 4)->text() == "yes");
        shot(w, "GT-06_lecturer_assignments.png");
        end("Lecturer creates an assignment with a brief; an empty title is refused");

        begin("GT-07");
        set(w, "title", "Project 2"); set(w, "brief", "nosuch.pdf");
        get<QDateTimeEdit>(w, "deadline")->setDateTime(QDateTime::currentDateTime().addDays(6));
        click(w, "createAssignment"); note(last());
        CHECK(last().contains("created") && last().contains("not attached"));
        end("A wrong brief path says the assignment was created without a brief (D-09, GUI side)");
    }

    begin("GT-08");
    for (const char* who : {"itu", "neo"}) {
        auto win = GuiApp::windowFor(s, login(s, who, "Password1"));
        QWidget* w = win.get();
        CHECK(dynamic_cast<StudentWindow*>(w));
        CHECK(get<QTableWidget>(w, "toDo")->rowCount() == 2);
        if (std::string(who) == "itu") shot(w, "GT-08_student_home.png");
        tab(w, 1);
        selectRowWith(w, "assignments", 2, "Project 1");
        set(w, "file", "\"missing.txt\""); click(w, "submit");
        if (std::string(who) == "itu") { note(last()); CHECK(last().contains("File not found")); }
        selectRowWith(w, "assignments", 2, "Project 1");
        set(w, "file", "\"" + QDir::currentPath() + "/essay.txt\""); click(w, "submit");
        if (std::string(who) == "itu") { note(last()); shot(w, "GT-08_student_submitted.png"); }
        CHECK(last().contains("on time"));
    }
    end("Students see their to-do list and submit; a missing file is refused");

    begin("GT-09");
    {
        auto win = GuiApp::windowFor(s, login(s, "thabo", "Password1"));
        QWidget* w = win.get();
        tab(w, 1);
        pick(w, "extensionAssignment", "Project 1");
        get<QSpinBox>(w, "student")->setValue(4);
        set(w, "extensionReason", "Medical");
       const QDateTime originalDeadline =
    QDateTime::currentDateTime().addDays(5);

    get<QDateTimeEdit>(w, "newDeadline")->setDateTime(
    originalDeadline.addDays(15));
        click(w, "grant"); note(last()); CHECK(last().contains("14 days"));
        get<QDateTimeEdit>(w, "newDeadline")->setDateTime(
    originalDeadline.addDays(3));

set(w, "extensionReason", "Medical");

click(w, "grant");
note(last());
CHECK(last().startsWith("INFO"));

        begin("GT-10");
        tab(w, 2);
        pick(w, "markingAssignment", "Project 1");
        CHECK(get<QTableWidget>(w, "submissions")->rowCount() == 2);
        selectRowWith(w, "submissions", 1, "4");
        get<QDoubleSpinBox>(w, "rawMark")->setValue(70); set(w, "feedback", "Good structure");
        click(w, "mark"); note(last()); CHECK(last().contains("70.0"));
        selectRowWith(w, "submissions", 1, "5");
        get<QDoubleSpinBox>(w, "rawMark")->setValue(60); set(w, "feedback", "Needs more tests");
        click(w, "mark");
        selectRowWith(w, "submissions", 1, "5");
        get<QDoubleSpinBox>(w, "rawMark")->setValue(62); set(w, "amendReason", ""); click(w, "amend");
        note(last()); CHECK(last().contains("reason"));
        set(w, "amendReason", "Missed a page"); click(w, "amend"); note(last()); CHECK(last().contains("62.0"));
        selectRowWith(w, "submissions", 1, "4");
        click(w, "openFile"); note(last()); CHECK(last().startsWith("OPEN: ") && last().endsWith("sub1_v1.txt"));
        shot(w, "GT-10_lecturer_marking.png");
        end("Lecturer opens the file, marks, and amends a mark only with a reason");
    }

    begin("GT-11");
    {
        auto win = GuiApp::windowFor(s, login(s, "itu", "Password1"));
        QWidget* w = win.get(); tab(w, 1);
        selectRowWith(w, "assignments", 2, "Project 1");
        auto* t = get<QTableWidget>(w, "assignments"); const int r = selectedRow(t);
        note(t->item(r, 4)->text());
        CHECK(t->item(r, 4)->text() == "Being marked");
        CHECK(t->item(r, 6)->text().isEmpty());
    }
    end("Before release the student sees 'Being marked' and no mark (BR-07)");

    begin("GT-12");
    {
        auto win = GuiApp::windowFor(s, login(s, "mpho", "Password1"));
        QWidget* w = win.get();
        CHECK(dynamic_cast<HodWindow*>(w));
        selectRowWith(w, "hodAssignments", 2, "Project 1");
        CHECK(get<QTableWidget>(w, "markedWork")->rowCount() == 2);
        click(w, "release"); note(last()); CHECK(last().contains("not moderated"));
        click(w, "approveAll"); note(last()); CHECK(last().contains("2"));
        selectRowWith(w, "markedWork", 1, "5");
        get<QDoubleSpinBox>(w, "finalMark")->setValue(65); set(w, "adjustReason", "");
        click(w, "adjust"); note(last()); CHECK(last().startsWith("ERROR"));
        set(w, "adjustReason", "Consistent with the moderation sample"); click(w, "adjust");
        shot(w, "GT-12_hod_moderation.png");
        click(w, "release"); note(last()); CHECK(last().contains("released"));
        end("HoD: release refused before moderation; approve, adjust with a reason, release");

        begin("GT-13");
        tab(w, 1);
        const int rows = get<QTableWidget>(w, "auditTable")->rowCount();
        note(QString::number(rows) + " audit rows");
        CHECK(rows == 4);  // two first marks, the lecturer's amendment, the HoD's adjustment
        shot(w, "GT-13_hod_audit.png");
        end("HoD's audit trail shows every mark change with who, when and why");
    }

    begin("GT-14");
    {
        auto win = GuiApp::windowFor(s, login(s, "itu", "Password1"));
        QWidget* w = win.get(); tab(w, 1);
        selectRowWith(w, "assignments", 2, "Project 1");
        auto* t = get<QTableWidget>(w, "assignments"); const int r = selectedRow(t);
        note(t->item(r, 4)->text() + " / " + t->item(r, 6)->text());
        CHECK(t->item(r, 4)->text() == "Released: 70.0 / 100");
        CHECK(t->item(r, 6)->text() == "Good structure");
        shot(w, "GT-14_student_released.png");
    }
    end("After release the student sees the mark and feedback");

    begin("GT-15");
    {
        auto win = GuiApp::windowFor(s, login(s, "admin", "Password1"));
        QWidget* w = win.get();
        tab(w, 4); set(w, "backupFolder", "backups/gui1"); click(w, "backUp"); note(last());
        CHECK(QFile::exists("backups/gui1/lms.db"));
        shot(w, "GT-15_admin_backup.png");
        tab(w, 0); selectRowWith(w, "users", 1, "neo"); click(w, "deactivateUser");
    }
    CHECK(!login(s, "neo", "Password1", &err)); note("neo: " + err);
    end("Administrator backs up the system and deactivates a user, who can then not log in");

    begin("GT-16");
    {
        ChangePasswordDialog d(s.auth, 4);
        auto edits = d.findChildren<QLineEdit*>();
        edits[0]->setText("Password1"); edits[1]->setText("NewPassword9"); edits[2]->setText("NewPassword9");
        d.findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
        note(last());
        CHECK(login(s, "itu", "NewPassword9") != nullptr);
        CHECK(!login(s, "itu", "Password1"));
    }
    end("A user changes their password; only the new one works");

    std::cout << "\nGUI acceptance checks failed: " << fails << "\n";
    return fails;
}
