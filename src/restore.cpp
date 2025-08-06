// restore.cpp

#include "restore.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QProgressBar>
#include <QDialog>
#include <QVBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QCloseEvent>

class InterruptibleProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit InterruptibleProgressDialog(bool &abortFlag, QWidget *parent = nullptr)
        : QDialog(parent), aborted(abortFlag) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            aborted = true;
            close();
        } else {
            QDialog::keyPressEvent(event);
        }
    }

    void closeEvent(QCloseEvent *event) override {
        aborted = true;
        QDialog::closeEvent(event);
    }

private:
    bool &aborted;
};

Restore::Restore(QObject *parent) : QObject(parent) {}

void Restore::run(QString fileName, const QString &bkp_prefix, const QString &bkp_host, const QString &bkp_schema, QWidget *parent) {

    QFile fileLog(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/restore.log");
    if (!fileLog.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(parent, "Error", "Failed to open file for writing: " + fileLog.errorString());
        qDebug() << "Error opening file:" << fileLog.errorString();
        return;
    }
    QTextStream out(&fileLog);

    if (fileName == "")
    {
        fileName = QFileDialog::getOpenFileName(
        parent,
        "Open query file",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "SQL files (*.sql);;All files (*)"
        );
        showMessage = true;
    }

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(parent, "Error", "Unable to open the backup file.");
        return;
    }

    bool aborted = false;
    InterruptibleProgressDialog progressDialog(aborted, parent);
    progressDialog.setWindowTitle("Restoring backup...");
    QVBoxLayout *layout = new QVBoxLayout(&progressDialog);
    QProgressBar *progressBar = new QProgressBar(&progressDialog);
    progressBar->setTextVisible(true);
    layout->addWidget(progressBar);
    progressDialog.setMinimumSize(400, 80);
    progressDialog.setModal(true);
    progressDialog.show();
    QApplication::processEvents();

    QTextStream in(&file);
    QStringList sqlCommands;
    QString currentLine;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("--"))
            continue;

        currentLine += line + " ";

        if (line.endsWith(";")) {
            sqlCommands << currentLine.trimmed();
            currentLine.clear();
        }
    }

    file.close();
    progressBar->setRange(0, sqlCommands.size());
    QSqlDatabase db = QSqlDatabase::database(bkp_prefix + bkp_host);
    if (!db.isOpen()) {
        QMessageBox::critical(parent, "Error", "Database connection is not open.");
        return;
    }

    QSqlQuery query(db);
    int i = 0;
    progressBar->setFormat("%v / %m");

    for (const QString &cmd : sqlCommands) {
        if (aborted)
            break;

        if (!query.exec(cmd)) {
            qWarning() << "Command error:" << cmd;
            qWarning() << query.lastError().text();
            out << "Command error:" << cmd << "\n";
            out << query.lastError().text() << "\n\n";
        }

        i++;
        progressBar->setValue(i);
        QApplication::processEvents();
    }

    fileLog.close();
    progressDialog.accept();
    if (showMessage)
    {
        if (aborted) {
            QMessageBox::information(parent, "Cancelled", "Backup restoration was aborted by the user.");
        } else {
            QMessageBox::information(parent, "Done", "Backup restored successfully.");
        }
    }
}

#include "restore.moc"
