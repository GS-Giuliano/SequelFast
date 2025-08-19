#include "texteditcompleter.h"

TextEditCompleter::TextEditCompleter(QWidget* parent)
    : QTextEdit(parent), c(nullptr)
{
}

void TextEditCompleter::setCompleter(QCompleter* completer)
{
    if (c) {
        disconnect(c, nullptr, this, nullptr);
    }

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->setMaxVisibleItems(10);
    connect(c, QOverload<const QString&>::of(&QCompleter::activated),
            this, &TextEditCompleter::insertCompletion);
}

QCompleter* TextEditCompleter::completer() const
{
    return c;
}

void TextEditCompleter::insertCompletion(const QString& completion)
{
    if (c && c->widget() != this)
        return;

    QTextCursor tc = textCursor();
    QString currentWord = getCurrentWord();
    if (currentWord.isEmpty()) {
        currentWord = c ? c->completionPrefix() : QString();
        qDebug() << "Palavra atual vazia, usando completionPrefix:" << currentWord;
    }

    qDebug() << "Inserting completion:" << completion << ", current word:" << currentWord;

    tc.movePosition(QTextCursor::StartOfWord);
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    tc.insertText(completion);
    setTextCursor(tc);

    qDebug() << "Text after insertion:" << toPlainText();
}

QString TextEditCompleter::getCurrentWord() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TextEditCompleter::keyPressEvent(QKeyEvent* e)
{
    this->setStyleSheet("");
    if (c && c->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Tab:
            e->accept();
            c->popup()->hide();
            if (!c->currentCompletion().isEmpty()) {
                insertCompletion(c->currentCompletion());
            }
            return;
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Home:
        case Qt::Key_End:
            c->popup()->hide();
            QTextEdit::keyPressEvent(e);
            return;
        case Qt::Key_Escape:
            e->accept();
            c->popup()->hide();
            return;
        default:
            break;
        }
    }

    QTextEdit::keyPressEvent(e);

    if (!c || e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift || e->key() == Qt::Key_Alt)
        return;

    QString currentWord = getCurrentWord();
    if (currentWord.length() < 2)
        return;

    QTextCursor tc = textCursor();
    QString blockText = tc.block().text().left(tc.positionInBlock());

    QRegularExpression commentRegex(R"((?:--.*$|/\*.*?\*/))");
    QRegularExpression stringRegex(R"('[^']*')");
    if (commentRegex.match(blockText).hasMatch() || stringRegex.match(blockText).hasMatch()) {
        qDebug() << "Dentro de comentário ou string, ignorando autocompletar";
        return;
    }

    c->setCompletionPrefix(currentWord);
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0) + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);
}

void TextEditCompleter::focusInEvent(QFocusEvent* e)
{
    if (c)
        c->setWidget(this);
    QTextEdit::focusInEvent(e);
}

// --------- SOMENTE TEXTO PURO AO COLAR / ARRASTAR ---------

bool TextEditCompleter::canInsertFromMimeData(const QMimeData* source) const
{
    // Aceita inserir apenas se houver texto (ignora html, imagens, urls, etc.)
    return source && source->hasText();
}

void TextEditCompleter::insertFromMimeData(const QMimeData* source)
{
    if (!source) return;

    // Insere sempre como texto puro
    insertPlainText(source->text());
}
