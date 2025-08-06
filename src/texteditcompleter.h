#ifndef TEXTEDITCOMPLETER_H
#define TEXTEDITCOMPLETER_H

#include <QTextEdit>
#include <QCompleter>
#include <QTextBlock> // Adicionado para suportar QTextBlock

class TextEditCompleter : public QTextEdit
{
    Q_OBJECT

public:
    explicit TextEditCompleter(QWidget *parent = nullptr);
    void setCompleter(QCompleter *completer);
    QCompleter *completer() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private slots:
    void insertCompletion(const QString &completion);

private:
    QString getCurrentWord() const;
    QCompleter *c;
};

#endif // TEXTEDITCOMPLETER_H
