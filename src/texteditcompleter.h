#pragma once

#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QCompleter>
#include <QDebug>
#include <QKeyEvent>
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

private slots:
    void insertCompletion(const QString& completion);

private:
    QString getCurrentWord() const;
    QCompleter* c;
};


