#ifndef MACROINPUTDIALOG_H
#define MACROINPUTDIALOG_H

#include <QDialog>
#include <QMap>
#include <QWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>

struct MacroField {
    QString name;
    QString type = "string";
    QString fullMatch;
};

class MacroInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit MacroInputDialog(const QVector<MacroField> &fields, QWidget *parent = nullptr);
    QMap<QString, QVariant> getValues() const;

private:
    QMap<QString, QWidget *> inputs;
};

#endif // MACROINPUTDIALOG_H
