#pragma once

#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QDebug>
#include <QKeyEvent>
#include <QMimeData>        // <-- ADICIONE
#include <QScrollBar>
#include <QTextBlock>
#include <QTextEdit>

class TextEditCompleter : public QTextEdit
{
    Q_OBJECT

public:
    explicit TextEditCompleter(QWidget* parent = nullptr);
    void setCompleter(QCompleter* completer);
    QCompleter* completer() const;

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;

    // Forçar colagem como texto puro (cobre paste e drag&drop)
    bool canInsertFromMimeData(const QMimeData* source) const override;
    void insertFromMimeData(const QMimeData* source) override;

private slots:
    void insertCompletion(const QString& completion);

private:
    QString getCurrentWord() const;
    QCompleter* c;
};
