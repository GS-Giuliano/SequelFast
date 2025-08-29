// SafetyLinterHandler.h
#pragma once
#include <QObject>
#include <QRegularExpression>
#include <QMessageBox>

class QueryHandler {
public:
    virtual ~QueryHandler() = default;
    void setNext(QueryHandler* n) { next_ = n; }

    // retorna true para continuar o pipeline; false para abortar
    virtual bool handle(QString& sql, QWidget* parent) {
        return next_ ? next_->handle(sql, parent) : true;
    }

private:
    QueryHandler* next_ = nullptr;
};

// ---------------------------------------------

class SafetyLinterHandler : public QueryHandler {
public:
    bool handle(QString& sql, QWidget* parent) override
    {
        // 1) percorre statements separados por ';'
        const QStringList stmts = splitStatements(sql);

        for (const QString& sRaw : stmts) {
            const QString s = sRaw.trimmed();
            if (s.isEmpty())
                continue;

            // 2) só nos importam DELETE / UPDATE
            if (!startsWithKeyword(s, "DELETE") && !startsWithKeyword(s, "UPDATE"))
                continue;

            // 3) “mascara” strings e comentários para não detectar WHERE dentro de aspas
            const QString masked = maskStringsAndComments(s);

            // 4) se não tiver WHERE ao “nível do statement”, pedir confirmação
            if (!containsWord(masked, "WHERE")) {
                const QString op = startsWithKeyword(s, "DELETE") ? "DELETE" : "UPDATE";

                const QString snippet = elide(s, 220);
                const QString msg = QString::fromUtf8(
                    "You are about to execute a %1 **without a WHERE**.\n\n"
                    "Statement:\n%2\n\n"
                    "This may affect many rows. Do you want to continue?"
                ).arg(op, snippet);

                auto ret = QMessageBox::question(parent,
                                                "Security confirmation",
                                                msg,
                                                QMessageBox::Yes | QMessageBox::No,
                                                QMessageBox::No);

                if (ret != QMessageBox::Yes)
                    return false; // ABORTA o pipeline
            }
        }

        // 5) segue o pipeline
        return QueryHandler::handle(sql, parent);
    }

private:
    // --- helpers ---

    static bool startsWithKeyword(const QString& s, const char* kw)
    {
        QRegularExpression re(QStringLiteral("^\\s*%1\\b").arg(QLatin1String(kw)),
                              QRegularExpression::CaseInsensitiveOption);
        return re.match(s).hasMatch();
    }

    static bool containsWord(const QString& s, const char* word)
    {
        QRegularExpression re(QStringLiteral("\\b%1\\b").arg(QLatin1String(word)),
                              QRegularExpression::CaseInsensitiveOption);
        return re.match(s).hasMatch();
    }

    // Remove conteúdo de strings e comentários, substituindo por espaços,
    // para que a busca por WHERE não acerte falso-positivos.
    static QString maskStringsAndComments(const QString& in)
    {
        QString out = in;

        // 1) comentários /* ... */
        static QRegularExpression blockComment(R"(/\*[\s\S]*?\*/)");
        out.replace(blockComment, " ");

        // 2) comentários de linha -- ... até \n
        static QRegularExpression dashComment(R"((--|#)[^\n]*)");
        out.replace(dashComment, " ");

        // 3) strings simples '...'
        static QRegularExpression singleQuotes(R"('(?:\\'|[^'])*')");
        out.replace(singleQuotes, " ");

        // 4) strings duplas "..." (por segurança)
        static QRegularExpression doubleQuotes(R"("(?:\\"|[^"])*")");
        out.replace(doubleQuotes, " ");

        return out;
    }

    // Divide por ';' ignorando espaços; não tenta ser parser SQL completo (suficiente para linter)
    static QStringList splitStatements(const QString& sql)
    {
        QString work = sql;
        // Uma abordagem simples: separa por ';' e ignora vazios
        QStringList parts = work.split(';', Qt::SkipEmptyParts);
        // Reanexa o ';' ao fim do trecho apenas para fins de exibição (opcional)
        for (QString& p : parts) p = p.trimmed();
        return parts;
    }

    // Corta texto longo para exibir no diálogo
    static QString elide(const QString& s, int max)
    {
        if (s.size() <= max) return s;
        return s.left(max - 3) + "...";
    }
};
