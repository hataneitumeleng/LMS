#include "gui/GuiCommon.h"

#include "data/DatabaseError.h"
#include "domain/Errors.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QTableWidget>
#include <QUrl>

#include <exception>

namespace lms {

MessageHooks& messageHooks() {
    static MessageHooks hooks{
        [](QWidget* parent, const QString& text) {
            QMessageBox::information(parent, "Moderated LMS", text);
        },
        [](QWidget* parent, const QString& text) {
            QMessageBox::warning(parent, "Moderated LMS", text);
        },
        [](QWidget* parent, const QString& question) {
            return QMessageBox::question(parent, "Please confirm", question) ==
                   QMessageBox::Yes;
        },
        [](const QString& path) { return QDesktopServices::openUrl(QUrl::fromLocalFile(path)); },
        [](QWidget* parent, const QString& title) {
            return QFileDialog::getOpenFileName(parent, title);
        }};
    return hooks;
}

void showInfo(QWidget* parent, const QString& text) { messageHooks().info(parent, text); }

void showError(QWidget* parent, const QString& text) { messageHooks().error(parent, text); }

bool confirm(QWidget* parent, const QString& question) {
    return messageHooks().confirm(parent, question);
}

QString chooseFile(QWidget* parent, const QString& title) {
    return messageHooks().chooseFile(parent, title);
}

void openSubmittedFile(QWidget* parent, const SubmissionVersion& version) {
    openStoredFile(parent, version.filePath);
}

void openStoredFile(QWidget* parent, const std::string& storedPath) {
    const QString path = QFileInfo(qs(storedPath)).absoluteFilePath();
    if (!QFileInfo::exists(path)) {
        showError(parent, "The file is missing from the submissions folder: " + path);
        return;
    }
    if (!messageHooks().openFile(path)) {
        showInfo(parent, "No program is set up to open this kind of file. It is saved here:\n" +
                             path);
    }
}

bool runAction(QWidget* parent, const std::function<void()>& action) {
    try {
        action();
        return true;
    } catch (const ValidationError& e) {
        showError(parent, qs(e.what()));
    } catch (const AuthError& e) {
        showError(parent, "Not allowed: " + qs(e.what()));
    } catch (const DatabaseError& e) {
        showError(parent, "The database refused this: " + qs(e.what()));
    } catch (const std::exception& e) {
        showError(parent, "Something went wrong: " + qs(e.what()));
    }
    return false;
}

QString qs(const std::string& text) { return QString::fromStdString(text); }

std::string str(const QString& text) { return text.trimmed().toStdString(); }

QString markText(const std::optional<double>& mark) {
    return mark ? QString::number(*mark, 'f', 1) : QString("-");
}

QDateTime toQt(TimePoint time) {
    return QDateTime::fromString(qs(toText(time)), "yyyy-MM-dd HH:mm:ss");
}

TimePoint fromQt(const QDateTime& dateTime) {
    return timeFromText(str(dateTime.toString("yyyy-MM-dd HH:mm:00")));
}

QTableWidget* makeTable(const QStringList& headers, QWidget* parent) {
    auto* table = new QTableWidget(0, static_cast<int>(headers.size()), parent);
    table->setHorizontalHeaderLabels(headers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    return table;
}

void fillTable(QTableWidget* table, const std::vector<QStringList>& rows) {
    table->clearSelection();
    table->setRowCount(static_cast<int>(rows.size()));
    for (int r = 0; r < static_cast<int>(rows.size()); ++r) {
        for (int c = 0; c < rows[r].size() && c < table->columnCount(); ++c) {
            table->setItem(r, c, new QTableWidgetItem(rows[r][c]));
        }
    }
    table->resizeColumnsToContents();
}

int selectedRow(const QTableWidget* table) {
    const QModelIndexList rows = table->selectionModel()->selectedRows();
    return rows.isEmpty() ? -1 : rows.first().row();
}

std::optional<int> selectedId(const QTableWidget* table) {
    const int row = selectedRow(table);
    if (row < 0) return std::nullopt;
    const QTableWidgetItem* item = table->item(row, 0);
    if (!item) return std::nullopt;
    return item->text().toInt();
}

std::optional<int> comboId(const QComboBox* combo) {
    if (combo->currentIndex() < 0) return std::nullopt;
    return combo->currentData().toInt();
}

}  // namespace lms
