#pragma once
#include "domain/SubmissionVersion.h"
#include "domain/TimeFormat.h"

#include <QDateTime>
#include <QString>
#include <QStringList>

#include <functional>
#include <optional>
#include <string>
#include <vector>

class QComboBox;
class QTableWidget;
class QWidget;

namespace lms {

// Small helpers shared by every window: messages, error handling, text, times and tables.

// The three kinds of message box, and opening a file in its normal program (Word, a PDF
// viewer...). A test can replace them so nobody has to click.
struct MessageHooks {
    std::function<void(QWidget*, const QString&)> info;
    std::function<void(QWidget*, const QString&)> error;
    std::function<bool(QWidget*, const QString&)> confirm;
    std::function<bool(const QString&)> openFile;
    std::function<QString(QWidget*, const QString&)> chooseFile;  // "" when cancelled
};
MessageHooks& messageHooks();

void showInfo(QWidget* parent, const QString& text);
void showError(QWidget* parent, const QString& text);
bool confirm(QWidget* parent, const QString& question);  // NFR-06

// Opens a stored file (a submission or a brief) with the program Windows uses for that type.
void openStoredFile(QWidget* parent, const std::string& storedPath);
void openSubmittedFile(QWidget* parent, const SubmissionVersion& version);
// Shows the "open file" box. Returns "" when the user cancels.
QString chooseFile(QWidget* parent, const QString& title);

// NFR-07: runs one button's action and shows any error in a message box, like Menu::run()
// does for the console. Returns true if the action finished without an error.
bool runAction(QWidget* parent, const std::function<void()>& action);

QString qs(const std::string& text);
std::string str(const QString& text);  // also removes spaces at both ends
QString markText(const std::optional<double>& mark);  // "70.0", or "-" when there is none

// Times are converted through the "YYYY-MM-DD HH:MM:SS" text of TimeFormat, so the
// windows and the console use exactly the same local time.
QDateTime toQt(TimePoint time);
TimePoint fromQt(const QDateTime& dateTime);

// A read-only table where the user picks whole rows. Column 0 always holds the id.
QTableWidget* makeTable(const QStringList& headers, QWidget* parent);
void fillTable(QTableWidget* table, const std::vector<QStringList>& rows);
int selectedRow(const QTableWidget* table);  // -1 when no row is chosen
std::optional<int> selectedId(const QTableWidget* table);
std::optional<int> comboId(const QComboBox* combo);  // the id stored with the chosen item

}  // namespace lms
