#include "two_checkbox_delegate.h"
#include <QApplication>
#include <QStyle>
#include <QMouseEvent>

TwoCheckboxDelegate::TwoCheckboxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void TwoCheckboxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    // Renderiza checkboxes apenas nas colunas 1 ("structure") e 2 ("data")
    if (index.column() == 1 || index.column() == 2) {
        QStyleOptionButton checkbox;
        // Centraliza o checkbox na célula
        int size = qMin(option.rect.width(), option.rect.height()) - 10;
        checkbox.rect = QRect(option.rect.center().x() - size / 2,
                              option.rect.center().y() - size / 2,
                              size, size);
        checkbox.state = index.data(Qt::UserRole).toBool() ? QStyle::State_On : QStyle::State_Off;
        checkbox.state |= QStyle::State_Enabled;

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkbox, painter);
    } else {
        // Renderiza texto normalmente para a coluna 0 ("table name")
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool TwoCheckboxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                      const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Lida com cliques apenas nas colunas 1 e 2
    if ((index.column() == 1 || index.column() == 2) && event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        int size = qMin(option.rect.width(), option.rect.height()) - 10;
        QRect checkboxRect(option.rect.center().x() - size / 2,
                           option.rect.center().y() - size / 2,
                           size, size);

        if (checkboxRect.contains(mouseEvent->pos())) {
            bool checked = index.data(Qt::UserRole).toBool();
            model->setData(index, !checked, Qt::UserRole);
            return true;
        }
    }
    return false;
}
