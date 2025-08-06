#ifndef MACROFORMATDIALOG_H
#define MACROFORMATDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>

class MacroFormatDialog : public QDialog {
    Q_OBJECT

public:
    explicit MacroFormatDialog(QWidget *parent = nullptr);
    QString resultMacro() const;

private slots:
    void onTypeChanged(const QString &type);

private:
    QLineEdit *labelEdit;
    QComboBox *typeCombo;

    QWidget *tableGroup;
    QFormLayout *groupLayout;

    QLineEdit *tableEdit;
    QLineEdit *keyEdit;
    QLineEdit *fieldEdit;

    QLabel *tableLabel;

    QString macroString;
};

#endif // MACROFORMATDIALOG_H
