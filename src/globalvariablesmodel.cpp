#include "globalvariablesmodel.h"

#include <QSqlError>
#include <QSqlQuery>

GlobalVariablesModel::GlobalVariablesModel(const QSqlDatabase& db, QObject* parent)
    : QAbstractListModel(parent)
{
    QSqlQuery query(db);
    if (query.exec(QStringLiteral("SELECT * FROM performance_schema.global_variables"))) {
        while (query.next()) {
            m_allRows.append(Row{query.value(0).toString(), query.value(1).toString()});
        }
    }
    m_filteredRows = m_allRows;
}

int GlobalVariablesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_filteredRows.size();
}

QVariant GlobalVariablesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredRows.size())
        return QVariant();

    const Row& row = m_filteredRows.at(index.row());
    switch (role) {
    case NameRole:
        return row.name;
    case ValueRole:
        return row.value;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> GlobalVariablesModel::roleNames() const
{
    return {
        {NameRole, "varName"},
        {ValueRole, "varValue"},
    };
}

void GlobalVariablesModel::setFilterText(const QString& text)
{
    if (m_filterText == text)
        return;
    m_filterText = text;
    applyFilter();
    emit filterTextChanged();
}

void GlobalVariablesModel::applyFilter()
{
    beginResetModel();
    if (m_filterText.isEmpty()) {
        m_filteredRows = m_allRows;
    } else {
        m_filteredRows.clear();
        for (const Row& row : std::as_const(m_allRows)) {
            if (row.name.contains(m_filterText, Qt::CaseInsensitive)
                || row.value.contains(m_filterText, Qt::CaseInsensitive)) {
                m_filteredRows.append(row);
            }
        }
    }
    endResetModel();
    emit countChanged();
}
