#pragma once

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QVector>

// Flat, filterable list of MySQL/MariaDB "SHOW GLOBAL VARIABLES"-style rows
// (name/value pairs), loaded once and filtered client-side so the QML
// Statistics screen can bind a plain ListView to it without needing a
// QSqlQueryModel + QSortFilterProxyModel column dance in QML.
class GlobalVariablesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(int count READ rowCountProp NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ValueRole,
    };

    explicit GlobalVariablesModel(const QSqlDatabase& db, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString& text);

    int rowCountProp() const { return rowCount(); }

signals:
    void filterTextChanged();
    void countChanged();

private:
    struct Row { QString name; QString value; };

    void applyFilter();

    QVector<Row> m_allRows;
    QVector<Row> m_filteredRows;
    QString m_filterText;
};
