#pragma once

#include <QAbstractListModel>
#include "backup.h"

class TwoCheckboxListModel : public QAbstractListModel {
    Q_OBJECT
public:
    QVector<ItemData> items;
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return items.size();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= items.size()) return {};
        const auto &item = items[index.row()];
        if (role == Qt::DisplayRole) return item.label;
        if (role == Qt::UserRole) return item.checkA;
        if (role == Qt::UserRole + 1) return item.checkB;
        return {};
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (!index.isValid() || index.row() >= items.size()) return false;
        auto &item = items[index.row()];
        if (role == Qt::UserRole) item.checkA = value.toBool();
        else if (role == Qt::UserRole + 1) item.checkB = value.toBool();
        else return false;
        emit dataChanged(index, index);
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
};


