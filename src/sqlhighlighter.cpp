#include "sqlhighlighter.h"
#include "functions.h"

extern QString currentTheme;

SqlHighlighter::SqlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    Rule rule;

    // Palavras-chave SQL (azul escuro e negrito)
    QTextCharFormat keywordFormat;
    if (currentTheme == "light")
        keywordFormat.setForeground(Qt::darkBlue);
    else
        keywordFormat.setForeground(QBrush(QColor("#A0A0FF")));
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywords = {
        "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bINSERT\\b", "\\bINTO\\b",
        "\\bVALUES\\b", "\\bUPDATE\\b", "\\bDELETE\\b", "\\bJOIN\\b", "\\bLEFT\\b",
        "\\bRIGHT\\b", "\\bINNER\\b", "\\bOUTER\\b", "\\bORDER\\b", "\\bBY\\b",
        "\\bGROUP\\b", "\\bLIMIT\\b", "\\bOFFSET\\b", "\\bAS\\b", "\\bDISTINCT\\b",
        "\\bAND\\b", "\\bOR\\b", "\\bNOT\\b", "\\bNULL\\b", "\\bIS\\b", "\\bCASE\\b",
        "\\bSHOW\\b","\\bDATABASE\\b","\\bTABLE\\b","\\bDESCRIBE\\b","\\bSET\\b",
        "\\bLIKE\\b","\\bTRUNCATE\\b"

    };

    for (const QString &word : keywords) {
        rule.pattern = QRegularExpression(word, QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        rules.append(rule);
    }

    // Funções SQL (laranja)
    QTextCharFormat functionFormat;
    if (currentTheme == "light")
        functionFormat.setForeground(Qt::darkRed);
    else
        functionFormat.setForeground(Qt::red);

    QStringList functions = {
        "\\bMIN\\b", "\\bMAX\\b", "\\bCOUNT\\b", "\\bCONCAT\\b", "\\bAVG\\b",
        "\\bSUM\\b", "\\bNOW\\b", "\\bCURDATE\\b", "\\bCURTIME\\b",
        "\\bDAY\\b", "\\bMONTH\\b","\\bYEAR\\b","\\bMONTHNAME\\b",
        "\\bHOUR\\b","\\bMINUTE\\b","\\bSECOND\\b","\\bMICROSECOND\\b",
        "\\bDAYOFMONTH\\b","\\bDAYOFWEEK\\b","\\bDAYOFYEAR\\b",
        "\\bDEFAULT\\b","\\bDIV\\b","\\bEXISTS\\b","\\bFIELD\\b",
        "\\bFIND_IN_SET\\b","\\bFIRST_VALUE\\b","\\bFLOOR\\b",
        "\\bFORMAT_BYTES\\b","\\bGROUP_CONCAT\\b",
        "\\bDATE\\b", "\\bDATE_ADD\\b", "\\bDATE_FORMAT\\b","\\bDATE_SUB\\b",
        "\\bDATEDIFF\\b", "\\bIFNULL\\b", "\\bCOALESCE\\b",
        "\\bLENGTH\\b", "\\bSUBSTRING\\b", "\\bROUND\\b", "\\bABS\\b",
        "\\bMOD\\b", "\\bADDDATE\\b", "\\bADDTIME\\b","\\bASCII\\b","\\bBINARY\\b",
        "\\bAES_DECRYPT\\b","\\bAES_ENCRYPT\\b",
        "\\bCAST\\b","\\bCEIL\\b","\\bCEILING\\b",
        "\\bCHAR_LENGTH\\b","\\bCHARSET\\b","\\bCOALESCE\\b","\\bCONCAT_WS\\b",
        "\\bCONVERT_TZ\\b","\\bCONVERT\\b","\\bCONV\\b",
        "\\bCURRENT_DATA\\b","\\bCURRENT_ROLE\\b","\\bCURRENT_TIME\\b","\\bCURRENT_USER\\b",
        "\\bCURRENT_TIMESTAMP\\b","\\bIF\\b","\\bIFNULL\\b","\\bIN\\b",
        "\\bLCASE\\b","\\bLOWER\\b","\\bUCASE\\b","\\bUPPER\\b",
        "\\bLPAD\\b","\\bRPAD\\b","\\bLTRIM\\b","\\bRTRIM\\b",
        "\\bMD5\\b","\\bSUBSTRING\\b","\\bSUBSTRING_INDEX\\b","\\bEXISTS\\b","\\bRAND\\b","\\bROUND\\b",
        "\\bSUBDATE\\b","\\bSUBTIME\\b","\\bSYSDATE\\b","\\bTIMESTAMP\\b",
        "\\bWEEK\\b","\\bWEEKDAY\\b","\\bWEEKOFYEAR\\b"
    };

    for (const QString &func : functions) {
        rule.pattern = QRegularExpression(func, QRegularExpression::CaseInsensitiveOption);
        rule.format = functionFormat;
        rules.append(rule);
    }


    // Identificadores entre `backticks` (tabelas, colunas)
    QTextCharFormat backtickFormat;
    backtickFormat.setForeground(QColor("#007777"));  // azul petróleo
    rule.pattern = QRegularExpression("`[^`]*`");
    rule.format = backtickFormat;
    rules.append(rule);

    // Números (int e float)
    QTextCharFormat numberFormat;
    if (currentTheme == "light")
        numberFormat.setForeground(QColor("#6A0DAD"));  // roxo escuro
    else
        numberFormat.setForeground(QColor("#9A3DDD"));  // roxo claro
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    rules.append(rule);

    // Strings entre aspas simples (verde escuro)
    QTextCharFormat stringFormat;
    stringFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("'[^']*'");
    rule.format = stringFormat;
    rules.append(rule);

    // Comentários SQL iniciados por "--" (cinza)
    QTextCharFormat commentFormat;
    if (currentTheme == "light")
        commentFormat.setForeground(Qt::darkGray);
    else
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
