#include "sqlhighlighter.h"

SqlHighlighter::SqlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    Rule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywords = {
        "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bINSERT\\b", "\\bINTO\\b",
        "\\bVALUES\\b", "\\bUPDATE\\b", "\\bDELETE\\b", "\\bJOIN\\b", "\\bLEFT\\b",
        "\\bRIGHT\\b", "\\bINNER\\b", "\\bOUTER\\b", "\\bORDER\\b", "\\bBY\\b",
        "\\bGROUP\\b", "\\bLIMIT\\b", "\\bOFFSET\\b", "\\bAS\\b", "\\bDISTINCT\\b",
        "\\bAND\\b", "\\bOR\\b", "\\bNOT\\b", "\\bNULL\\b", "\\bIS\\b"
    };

    for (const QString &word : keywords) {
        rule.pattern = QRegularExpression(word, QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        rules.append(rule);
    }

    // Strings entre aspas simples
    QTextCharFormat stringFormat;
    stringFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("'[^']*'");
    rule.format = stringFormat;
    rules.append(rule);

    // Comentários SQL: -- até o final da linha
    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("--[^\n]*");
    rule.format = commentFormat;
    rules.append(rule);
}

void SqlHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
