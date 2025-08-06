#ifndef MACROINPUTDIALOG_H
#define MACROINPUTDIALOG_H

#include <QDialog>
#include <QMap>
#include <QWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlField>

struct MacroField {
    QString name;
    QString type;
    QString table;
    QString key;
    QString display;
    QString order;
    QString full;  // macro completa como apareceu
};


class MacroInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit MacroInputDialog(const QVector<MacroField> &fields, const QString &sql_host, const QString &sql_schema, QWidget *parent = nullptr);
    QMap<QString, QVariant> getValues() const;

private:
    QMap<QString, QWidget *> inputs;
};

#endif // MACROINPUTDIALOG_H
