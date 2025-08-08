#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

class MacroFormatDialog : public QDialog {
    Q_OBJECT

public:
    explicit MacroFormatDialog(QWidget* parent = nullptr);
    QString resultMacro() const;

private slots:
    void onTypeChanged(const QString& type);

private:
    QLineEdit* labelEdit;
    QComboBox* typeCombo;

    QWidget* tableGroup;
    QFormLayout* groupLayout;

    QLineEdit* tableEdit;
    QLineEdit* keyEdit;
    QLineEdit* fieldEdit;

    QLabel* tableLabel;

    QString macroString;
};


