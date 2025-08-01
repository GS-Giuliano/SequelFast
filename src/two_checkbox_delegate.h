#ifndef TWO_CHECKBOX_DELEGATE_H
#define TWO_CHECKBOX_DELEGATE_H

#include <QStyledItemDelegate>

/**
 * Delegate personalizado para renderizar checkboxes nas colunas "structure" e "data" de um QTableView.
 * Aplica-se apenas às células das colunas 1 e 2, deixando o cabeçalho inalterado.
 */
class TwoCheckboxDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TwoCheckboxDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif // TWO_CHECKBOX_DELEGATE_H
