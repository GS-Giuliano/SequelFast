#include "sql.h"
#include "ui_sql.h"
#include "sqlhighlighter.h"
#include "texteditcompleter.h"
#include "mainwindow.h"

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include <QUndoStack>
#include <QUndoCommand>

#include "SafetyLinterHandler.h"

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;
extern QString currentTheme;
extern int pref_sql_limit;
extern int pref_table_row_height;
extern int pref_table_font_size;
extern int pref_sql_font_size;

class SmartSortProxy : public QSortFilterProxyModel {
public:
    explicit SmartSortProxy(QObject* parent=nullptr)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
        setSortCaseSensitivity(Qt::CaseInsensitive);
    }

    void setSqlRecord(const QSqlRecord& rec) { rec_ = rec; }

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override
    {
        // Se estiver pedindo para ordenar por um role específico (ex.: OriginalRowRole), respeite-o
        if (sortRole() != Qt::EditRole) {
            const QVariant L = sourceModel()->data(left,  sortRole());
            const QVariant R = sourceModel()->data(right, sortRole());
            return compareVariants(L, R);
        }

        const int col = left.column();

        auto getTyped = [&](const QModelIndex& idx) -> QVariant {
            QVariant v = sourceModel()->data(idx, Qt::EditRole);
            if (!v.isValid()) v = sourceModel()->data(idx, Qt::DisplayRole);
            return v;
        };

        QVariant l = getTyped(left);
        QVariant r = getTyped(right);

        // Tipo declarado no record do SELECT (pode ajudar)
        QVariant::Type declared = (col < rec_.count()) ? rec_.field(col).type() : QVariant::Invalid;

        // 1) Datas
        if (declared == QVariant::Date || declared == QVariant::DateTime || declared == QVariant::Time) {
            bool okL=false, okR=false;
            QDateTime dl = toDateTime(l, &okL);
            QDateTime dr = toDateTime(r, &okR);
            if (okL && okR) return dl < dr;
        }

        // 2) Numérico nativo
        if (declared == QVariant::Int || declared == QVariant::UInt ||
            declared == QVariant::LongLong || declared == QVariant::ULongLong ||
            declared == QVariant::Double)
        {
            bool okL=false, okR=false;
            double dl = toDouble(l, &okL);
            double dr = toDouble(r, &okR);
            if (okL && okR) return dl < dr;
        }

        // 3) “Numérico em string” (ex.: DECIMAL retorna QString)
        {
            bool okL=false, okR=false;
            double dl = toDouble(l, &okL);
            double dr = toDouble(r, &okR);
            if (okL && okR) return dl < dr;
        }

        // 4) Fallback: texto (locale-aware, case-insensitive)
        const QString sl = sourceModel()->data(left,  Qt::DisplayRole).toString();
        const QString sr = sourceModel()->data(right, Qt::DisplayRole).toString();
        return QString::localeAwareCompare(sl, sr) < 0;
    }

private:
    QSqlRecord rec_;

    static double toDouble(const QVariant& v, bool* okOut)
    {
        bool ok=false;
        double d = v.toDouble(&ok);
        if (!ok) {
            // tenta com locale do usuário
            d = QLocale().toDouble(v.toString(), &ok);
        }
        if (!ok) {
            // tenta locale "C" (ponto decimal)
            d = QLocale::c().toDouble(v.toString(), &ok);
        }
        if (okOut) *okOut = ok;
        return d;
    }

    static QDateTime toDateTime(const QVariant& v, bool* okOut)
    {
        bool ok = false;
        QDateTime dt;

        switch (v.type()) {
        case QVariant::Date: {
            const QDate d = v.toDate();
            if (d.isValid()) {
                // Qt 6: usar data + 00:00:00 (ou d.startOfDay())
                dt = QDateTime(d, QTime(0,0,0));
                ok = dt.isValid();
            }
            break;
        }
        case QVariant::Time: {
            const QTime t = v.toTime();
            if (t.isValid()) {
                // usar uma data fixa + hora
                dt = QDateTime(QDate(1970,1,1), t);
                ok = dt.isValid();
            }
            break;
        }
        case QVariant::DateTime: {
            dt = v.toDateTime();
            ok = dt.isValid();
            break;
        }
        default:
            // tenta ISO e formatos comuns
            dt = QDateTime::fromString(v.toString(), Qt::ISODate);
            ok = dt.isValid();
            if (!ok) {
                dt = QDateTime::fromString(v.toString(), "yyyy-MM-dd HH:mm:ss");
                ok = dt.isValid();
            }
            if (!ok) {
                // só data
                const QDate d = QDate::fromString(v.toString(), "yyyy-MM-dd");
                if (d.isValid()) {
                    dt = QDateTime(d, QTime(0,0,0));
                    ok = true;
                }
            }
            break;
        }

        if (okOut) *okOut = ok;
        return dt;
    }

    static bool compareVariants(const QVariant& L, const QVariant& R)
    {
        // tenta numérico
        bool okL=false, okR=false;
        double dl = toDouble(L, &okL);
        double dr = toDouble(R, &okR);
        if (okL && okR) return dl < dr;

        // tenta data/hora
        QDateTime tl = toDateTime(L, &okL);
        QDateTime tr = toDateTime(R, &okR);
        if (okL && okR) return tl < tr;

        // fallback texto
        return QString::localeAwareCompare(L.toString(), R.toString()) < 0;
    }
};

class CustomDelegate : public QStyledItemDelegate {
public:
    CustomDelegate(QObject* parent, const QSqlRecord& record)
        : QStyledItemDelegate(parent), currentRecord(record) {
    }
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QSqlField field = currentRecord.field(index.column());
        QVariant::Type type = field.type();

        if (type == QVariant::Date) {
            auto* editor = new QDateEdit(parent);
            editor->setCalendarPopup(true);
            editor->setDisplayFormat("yyyy-MM-dd");
            return editor;
        }

        if (type == QVariant::DateTime || type == QVariant::Time) {
            auto* editor = new QDateTimeEdit(parent);
            editor->setCalendarPopup(true);
            editor->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
            return editor;
        }

        if (type == QVariant::String) {
            const QString v = index.model()->data(index, Qt::EditRole).toString();
            if (v.size() > 50 || v.contains('\n')) {
                auto* editor = new QPlainTextEdit(parent);
                editor->setMinimumHeight(80);
                return editor;
            } else {
                // Caso contrário, editor de linha única
                auto* editor = new QLineEdit(parent);
                return editor;
            }
        }

        if (type == QVariant::Int || type == QVariant::UInt ||
            type == QVariant::LongLong || type == QVariant::ULongLong) {
            auto* editor = new QLineEdit(parent);
            editor->setValidator(new QIntValidator(editor));
            return editor;
        }

        if (type == QVariant::Double) {
            auto* editor = new QLineEdit(parent);
            auto* validator = new QDoubleValidator(editor);
            validator->setNotation(QDoubleValidator::StandardNotation);
            validator->setDecimals(10);
            editor->setValidator(validator);
            return editor;
        }

        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        const QString value = index.model()->data(index, Qt::EditRole).toString();
        const QSqlField field = currentRecord.field(index.column());
        const QVariant::Type type = field.type();

        if (type == QVariant::Date) {
            if (auto* dateEditor = qobject_cast<QDateEdit*>(editor)) {
                dateEditor->setDate(index.model()->data(index, Qt::EditRole).toDate());
                return;
            }
        }

        if (type == QVariant::DateTime || type == QVariant::Time) {
            if (auto* dtEditor = qobject_cast<QDateTimeEdit*>(editor)) {
                dtEditor->setDateTime(index.model()->data(index, Qt::EditRole).toDateTime());
                return;
            }
        }

        // Para QString, definimos no editor correspondente criado em createEditor
        if (auto* textEditor = qobject_cast<QPlainTextEdit*>(editor)) {
            textEditor->setPlainText(value);
            return;
        }
        if (auto* lineEdit = qobject_cast<QLineEdit*>(editor)) {
            lineEdit->setText(value);
            return;
        }

        QStyledItemDelegate::setEditorData(editor, index);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QSqlField field = currentRecord.field(index.column());
        QVariant::Type type = field.type();

        if (type == QVariant::Date) {
            if (auto* dateEditor = qobject_cast<QDateEdit*>(editor)) {
                model->setData(index, dateEditor->date().toString("yyyy-MM-dd"));
                return;
            }
        }

        if (type == QVariant::DateTime || type == QVariant::Time) {
            if (auto* dtEditor = qobject_cast<QDateTimeEdit*>(editor)) {
                model->setData(index, dtEditor->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
                return;
            }
        }

        if (type == QVariant::String) {
            if (auto* textEditor = qobject_cast<QPlainTextEdit*>(editor)) {
                model->setData(index, textEditor->toPlainText());
                return;
            }
            if (auto* lineEdit = qobject_cast<QLineEdit*>(editor)) {
                model->setData(index, lineEdit->text());
                return;
            }
        }

        if (auto* lineEdit = qobject_cast<QLineEdit*>(editor)) {
            model->setData(index, lineEdit->text());
            return;
        }

        QStyledItemDelegate::setModelData(editor, model, index);
    }

private:
    QSqlRecord currentRecord;
};

class UpdateCellCommand : public QUndoCommand {
public:
    UpdateCellCommand(Sql* self,
                      const QPersistentModelIndex& sourceIndex,
                      int idColumn,
                      QString fieldName,
                      QString oldValue,
                      QString newValue)
        : self(self),
          sourceIndex(sourceIndex),
          idColumn(idColumn),
          fieldName(std::move(fieldName)),
          oldValue(std::move(oldValue)),
          newValue(std::move(newValue))
    {
        setText(QString("Edit %1").arg(this->fieldName));
    }

    void redo() override { apply(newValue); }
    void undo() override { apply(oldValue); }

private:
    Sql* self;
    QPersistentModelIndex sourceIndex;  // índice no *source* (QStandardItemModel)
    int idColumn;
    QString fieldName, oldValue, newValue;

    void apply(const QString& value)
    {
        if (!sourceIndex.isValid()) return;

        auto* srcModel = qobject_cast<QStandardItemModel*>(self->tableProxy->sourceModel());
        if (!srcModel) return;

        // seta no modelo (EditRole + espelha Display/UserRole)
        srcModel->setData(sourceIndex, value, Qt::EditRole);
        srcModel->setData(sourceIndex, value, Qt::DisplayRole);
        srcModel->setData(sourceIndex, value, Qt::UserRole);

        // negrito para indicar “alterado”
        if (auto* item = srcModel->item(sourceIndex.row(), sourceIndex.column())) {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }

        // id da linha (coluna “id” já identificada na sua montagem)
        const QString idValue = srcModel->item(sourceIndex.row(), idColumn)
                                    ? srcModel->item(sourceIndex.row(), idColumn)->text()
                                    : QString();

        if (!idValue.isEmpty())
            self->handleTableData_edit_trigger(const_cast<QString&>(idValue),
                                               fieldName,
                                               const_cast<QString&>(value));
    }
};














Sql::Sql(const QString& host, const QString& schema, const QString& table,
         const QString& color, const QString& favName, const QString& favValue,
         const bool& run, QWidget* parent)
    : QMainWindow(parent), ui(new Ui::Sql)
{
    sql_host = host;
    sql_schema = schema;
    sql_table = table;
    sql_color = color;

    QString limit = getStringPreference("fav_limit");
    QString autoCommit = getStringPreference("pref_autocommit");

    pref_sql_limit = limit.toInt();

    ui->setupUi(this);

    QString param;
    QString queryStr;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    // no construtor, logo após ui->setupUi(this);
    ui->textQuery->setUndoRedoEnabled(true);

    // Ações de Undo/Redo com atalhos padrão (Ctrl+Z / Ctrl+Y ou Cmd+Z / Shift+Cmd+Z)
    auto *actTextUndo = new QAction(QIcon(":/icons/resources/arrow square left.svg"), tr("Undo"), this);
    actTextUndo->setShortcut(QKeySequence::Undo);
    actTextUndo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actTextUndo, &QAction::triggered, ui->textQuery, &QTextEdit::undo);

    auto *actTextRedo = new QAction(QIcon(":/icons/resources/arrow square right.svg"), tr("Redo"), this);
    actTextRedo->setShortcut(QKeySequence::Redo);
    actTextRedo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actTextRedo, &QAction::triggered, ui->textQuery, &QTextEdit::redo);

    // (Opcional) refletir disponibilidade habilitando/desabilitando os botões
    actTextUndo->setEnabled(false);
    actTextRedo->setEnabled(false);
    connect(ui->textQuery->document(), &QTextDocument::undoAvailable,
            actTextUndo, &QAction::setEnabled);
    connect(ui->textQuery->document(), &QTextDocument::redoAvailable,
            actTextRedo, &QAction::setEnabled);

    // (Opcional) adicionar na sua toolbar existente
    ui->toolBarQuery->addAction(actTextUndo);
    ui->toolBarQuery->addAction(actTextRedo);

    if (favValue != "")
    {
        queryStr = favValue;
        QStringList fav = favName.split("^");
        favoriteName = fav[5];
        this->setWindowTitle(favoriteName);
        connectMySQL(fav[1], this);
        dbMysqlLocal = QSqlDatabase::database("mysql_connection_" + sql_host);
        databaseName = fav[2];
    }
    else
    {
        this->setWindowTitle(schema + " • " + table);
        connectMySQL(sql_host, parent);
        dbMysqlLocal = QSqlDatabase::database("mysql_connection_" + sql_host);
        databaseName = sql_schema;
        param = sql_host + "^" + sql_schema + "^" + sql_table;
        queryStr = getStringPreference(param);
    }

    if (queryStr == "")
    {
        queryStr = "SELECT * FROM " + table + " LIMIT " + QString::number(pref_sql_limit) + ";";
    }

    QSqlQuery query(dbMysqlLocal);
    QString consulta = "USE " + databaseName + " ";
    if (!query.exec(consulta)) {
        qWarning() << "Erro ao selecionar banco de dados:" << query.lastError().text();
    }

    QApplication::restoreOverrideCursor();
    QApplication::processEvents();

    setInterfaceSize(0);

    QString style = "QTextEdit {background-color: " + getRgbFromColorName(sql_color) + "}";
    ui->textQuery->setStyleSheet(style);

    if (currentTheme == "light")
        ui->textQuery->setTextColor(QColor("black"));
    else
        ui->textQuery->setTextColor(QColor("white"));

    new SqlHighlighter(ui->textQuery->document());
    ui->textQuery->setText(queryStr);

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // QLabel* labelFavName = new QLabel(databaseName, this);
    QLabel* label = new QLabel("Every ", this);
    QLabel* labelSeconds = new QLabel("second(s) ", this);
    QLabel* labelTimes = new QLabel(" time(s)", this);
    label->setMargin(0);
    labelSeconds->setMargin(0);
    labelTimes->setMargin(0);

    spacer->setStyleSheet("QWidget {background-color: transparent}");

    edit = new QLineEdit(this);
    edit->setText("1");
    edit->setFixedWidth(50);
    edit->setPlaceholderText("Seconds");
    edit->setTextMargins(0, 0, 0, 0);
    QIntValidator* validator = new QIntValidator(0, 3600, this);
    edit->setValidator(validator);

    editTimes = new QLineEdit(this);
    editTimes->setText("5");
    editTimes->setFixedWidth(50);
    editTimes->setPlaceholderText("Times");
    editTimes->setTextMargins(0, 0, 0, 0);
    QIntValidator* validatorTimes = new QIntValidator(0, 99999, this);
    editTimes->setValidator(validatorTimes);

    button = new QPushButton(this);
    QIcon icone(":/icons/resources/clock.svg");
    button->setIcon(icone);
    button->setIconSize(QSize(16, 16));
    button->setCheckable(true);
    button->setChecked(false);

    connect(button, &QPushButton::clicked, this, &Sql::handleButton_clicked);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Sql::handleTimer_tick);

    // ui->toolBarQuery->addWidget(labelFavName);
    ui->toolBarQuery->addWidget(spacer);
    ui->toolBarQuery->addWidget(label);
    ui->toolBarQuery->addWidget(edit);
    ui->toolBarQuery->addWidget(labelSeconds);
    ui->toolBarQuery->addWidget(editTimes);
    ui->toolBarQuery->addWidget(labelTimes);
    ui->toolBarQuery->addWidget(button);

    ui->tableData->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableData, &QTableView::customContextMenuRequested,
            this, &Sql::show_context_menu);




    // --- Filtro na barra inferior (status bar) ---
    auto *filterLabel = new QLabel("Filter", this);
    auto *filterEdit  = new QLineEdit(this);
    filterEdit->setPlaceholderText("Filter");
    filterEdit->setClearButtonEnabled(true);
    filterEdit->setFixedWidth(120);

    // adiciona na barra inferior (lado direito)
    ui->toolBar->addWidget(filterLabel);
    ui->toolBar->addWidget(filterEdit);

    // quando o usuário digitar, aplica o filtro no proxy
    connect(filterEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        if (!tableProxy) return;                    // ainda não há dados/modelo
        tableProxy->setFilterKeyColumn(-1);         // filtra em todas as colunas
        QRegularExpression re(text, QRegularExpression::CaseInsensitiveOption);
        tableProxy->setFilterRegularExpression(re); // aplica regex
        showChart();
    });


    auto *limitLabel = new QLabel("Limit", this);
    limitEdit  = new QLineEdit(this);
    if (limit ==  "" || limit.toInt() == 0)
    {
        limit = "500";
    }
    pref_sql_limit = limit.toInt();
    limitEdit->setText(limit);
    limitEdit->setClearButtonEnabled(true);
    limitEdit->setFixedWidth(80);

    // adiciona na barra inferior (lado direito)
    ui->toolBar->addWidget(limitLabel);
    ui->toolBar->addWidget(limitEdit);

    // quando o usuário digitar, aplica o filtro no proxy
    connect(limitEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        if (text=="" || text.toInt() == 0)
        {
            limitEdit->setText("500");
            return;
        }
        setStringPreference("fav_limit",  text);
    });

    // --- Ordenação cíclica pelo cabeçalho ---
    ui->tableData->setSortingEnabled(true);
    ui->tableData->horizontalHeader()->setSortIndicatorShown(true);
    connect(ui->tableData->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &Sql::on_tableHeader_sectionClicked);


    ui->chartArea->setVisible(false);

    if (autoCommit == "1")
    {
        ui->actionAuto_commit->setChecked(true);
    }

    undoStack = new QUndoStack(this);

    // (opcional) atalhos padrão de undo/redo:
    auto actUndo = undoStack->createUndoAction(this, tr("Undo"));
    actUndo->setShortcut(QKeySequence::Undo);
    addAction(actUndo);

    auto actRedo = undoStack->createRedoAction(this, tr("Redo"));
    actRedo->setShortcut(QKeySequence::Redo);
    addAction(actRedo);




    QApplication::processEvents();
    if (run)
    {
        on_actionRun_triggered();
    }
}

Sql::~Sql()
{
    delete ui;
}

QVector<MacroField> Sql::extractFields(const QString& queryStr)
{
    QVector<MacroField> fields;
    QString maskedQuery = queryStr;
    QRegularExpression stringLiteralRegex(R"('(?:[^']|'')*')");
    QRegularExpressionMatchIterator stringIt = stringLiteralRegex.globalMatch(queryStr);

    while (stringIt.hasNext()) {
        QRegularExpressionMatch match = stringIt.next();
        int start = match.capturedStart();
        int length = match.capturedLength();
        maskedQuery.replace(start, length, QString(length, ' '));
    }

    QRegularExpression macroRegex(R"(~([A-Za-z0-9_çÇáàâãéèêíïóôõöúñÁÀÂÃÉÈÊÍÏÓÔÕÖÚÑ]+)(?:@([a-zA-Z_][a-zA-Z0-9_]*))?(?:~([A-Za-z0-9_%]+))?(?:~([A-Za-z0-9_]+))?(?:~([A-Za-z0-9_]+))?(?:~([A-Za-z0-9_]+))?(?=\b|\W))");

    QRegularExpressionMatchIterator it = macroRegex.globalMatch(maskedQuery);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        if (!match.hasMatch()) continue;

        MacroField field;
        field.name = match.captured(1);
        field.type = match.captured(2).isEmpty() ? "string" : match.captured(2).toLower();
        field.table = match.captured(3);
        field.key = match.captured(4);
        field.display = match.captured(5);
        field.order = match.captured(6);
        field.full = match.captured(0);
        fields.append(field);
    }
    return fields;
}

QString Sql::processQueryWithMacros(QString queryStr, QWidget* parent)
{
    auto fields = extractFields(queryStr);
    if (fields.isEmpty())
        return queryStr;

    MacroInputDialog dialog(fields, sql_host, sql_schema, parent);
    if (dialog.exec() == QDialog::Accepted) {
        QMap<QString, QVariant> values = dialog.getValues();
        for (const auto& field : fields) {
            QString value = values[field.name].toString();
            value.replace('\'', "''");
            if (field.type == "number")
            {
                queryStr.replace(field.full, QString("%1").arg(value));
            }
            else {
                queryStr.replace(field.full, QString("'%1'").arg(value));
            }
        }
    }
    return queryStr;
}

void Sql::setupSqlCompleter()
{
    QStringList keywords = {
        "FROM", "WHERE", "JOIN", "LEFT JOIN", "INNER JOIN", "RIGHT JOIN",
        "VALUES", "AS", "ON", "AND", "OR",
        "IN", "NULL", "LIMIT", "ORDER BY", "GROUP BY"
    };

    if (!sqlCompleterModel) {
        sqlCompleterModel = new QStringListModel(this);
    }
    sqlCompleterModel->setStringList(keywords);

    if (!sqlCompleter) {
        sqlCompleter = new QCompleter(sqlCompleterModel, this);
        sqlCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        sqlCompleter->setFilterMode(Qt::MatchContains);
    }

    ui->textQuery->setCompleter(sqlCompleter);

    connect(ui->textQuery, &QTextEdit::cursorPositionChanged, this, [=]() {
        QTextCursor cursor = ui->textQuery->textCursor();
        QString blockText = cursor.block().text().left(cursor.positionInBlock());

        QRegularExpression reFrom(R"(\bFROM\s+(\w+)(?:\s+(\w+))?\b)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression reTableAlias(R"((\w+)\.)", QRegularExpression::CaseInsensitiveOption);
        if (auto match = reFrom.match(blockText); match.hasMatch()) {
            QString table = match.captured(1);
            QString alias = match.captured(2);
            Q_UNUSED(alias);
            sqlCompleter->setModel(getColumnModel(table));
        }
        else if (auto match = reTableAlias.match(blockText); match.hasMatch()) {
            QString prefix = match.captured(1);
            QString table = prefix == tableAlias ? tableName : prefix;
            sqlCompleter->setModel(getColumnModel(table));
        }
        else {
            QStringList suggestions = keywords;
            QSqlQuery query(dbMysqlLocal);
            if (query.exec("SHOW TABLES")) {
                while (query.next()) {
                    suggestions << query.value(0).toString();
                }
            }
            suggestions.removeDuplicates();
            suggestions.sort(Qt::CaseInsensitive);
            sqlCompleterModel->setStringList(suggestions);
            sqlCompleter->setModel(sqlCompleterModel);
        }
    });
}

void Sql::handleButton_clicked()
{
    if (button->isChecked())
    {
        timer->start(edit->text().toInt() * 1000);
        edit->setEnabled(false);
        ui->textQuery->setEnabled(false);
    }
    else
    {
        if (timer->isActive())
        {
            edit->setEnabled(true);
            timer->stop();
            ui->textQuery->setEnabled(true);
        }
    }
}

QAbstractItemModel* Sql::getTableModel() const
{
    QStringList tables;
    QSqlQuery query(dbMysqlLocal);
    if (query.exec("SHOW TABLES")) {
        while (query.next()) {
            tables << query.value(0).toString();
        }
    }
    QStringListModel* model = new QStringListModel(tables, const_cast<Sql*>(this));
    return model;
}

QAbstractItemModel* Sql::getColumnModel(const QString& table) const
{
    QStringList columns;
    if (!table.isEmpty()) {
        QSqlQuery query(dbMysqlLocal);
        if (query.exec("SELECT * FROM " + table + " LIMIT 1")) {
            QSqlRecord record = query.record();
            for (int i = 0; i < record.count(); ++i) {
                columns << record.fieldName(i);
            }
        }
    }
    QStringListModel* model = new QStringListModel(columns, const_cast<Sql*>(this));
    return model;
}

void Sql::setInterfaceSize(int increase)
{
    if (increase > 0 && pref_sql_font_size < 30)
    {
        pref_sql_font_size++;
        pref_table_font_size++;
        pref_table_row_height += 2;
    }
    if (increase < 0 && pref_sql_font_size > 6)
    {
        pref_sql_font_size--;
        pref_table_font_size--;
        pref_table_row_height -= 2;
    }
    QFont fonte;
    fonte.setFamilies(QStringList()
                      << "Segoe UI"
                      << ".SF NS Text"
                      << "Ubuntu"
                      << "Cantarell"
                      << "Noto Sans"
                      << "Sans Serif");
    fonte.setPointSize(pref_table_font_size);
    ui->tableData->setFont(fonte);

    QFont fonteQuery;
    fonteQuery.setFamilies(QStringList() << "Consolas" << "Menlo" << "Courier New" << "Monospace");
    fonteQuery.setPointSize(pref_sql_font_size);
    fonteQuery.setStyleHint(QFont::Monospace);
    ui->textQuery->setFont(fonteQuery);

    ui->tableData->viewport()->update();
    ui->textQuery->viewport()->update();
}

void Sql::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        event->modifiers() & Qt::ControlModifier)
    {
        on_actionRun_triggered();
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}

void Sql::query2TableView(QTableView* tableView, const QString& queryStr, const QString& comando)
{
    // Modelo base com dados
    QStandardItemModel* model = new QStandardItemModel(tableView);
    QSqlQuery query(dbMysqlLocal);
    if (!query.exec(queryStr)) {
        statusMessage("Query error: " + query.lastError().text());
        ui->textQuery->setStyleSheet("QTextEdit { border: 1px solid red; }");
        qWarning() << "Erro na query e table:" << query.lastError().text();
        delete model;
        return;
    }
    log(queryStr);

    currentRecord = query.record();
    int colCount = currentRecord.count();
    idPosition = -1;
    hasId = false;

    for (int i = 0; i < colCount; ++i) {
        model->setHorizontalHeaderItem(i, new QStandardItem(currentRecord.fieldName(i)));
        if (currentRecord.fieldName(i) == "id") {
            hasId = true;
            idPosition = i;
        }
    }

    int originalRow = 0; // guarda ordem original
    while (query.next()) {
        QApplication::processEvents();
        QList<QStandardItem*> rowItems;
        for (int col = 0; col < colCount; ++col) {
            QVariant value = query.value(col);
            QVariant::Type type = currentRecord.field(col).type();

            QString displayText;

            switch (type) {
            case QVariant::Date:
                displayText = value.toDate().toString("yyyy-MM-dd");
                break;
            case QVariant::Time:
                displayText = value.toTime().toString("HH:mm:ss");
                break;
            case QVariant::DateTime:
                displayText = value.toDateTime().toString("yyyy-MM-dd HH:mm:ss");
                break;
            default:
                displayText = value.toString();
            }

            QStandardItem* item = new QStandardItem(displayText);
            item->setEditable(true);
            item->setData(value, Qt::EditRole);
            item->setData(displayText, Qt::DisplayRole);
            item->setData(displayText, Qt::UserRole);

            // grava ordem original (mesmo valor em todas colunas da linha)
            item->setData(originalRow, OriginalRowRole);

            switch (type) {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Double:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                break;
            case QVariant::Date:
            case QVariant::Time:
            case QVariant::DateTime:
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                break;
            default:
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                break;
            }

            rowItems << item;
        }
        model->appendRow(rowItems);
        ++originalRow;
    }

    // Encapa com proxy p/ ordenar sem perder ordem original
    if (!tableProxy) {
        // tableProxy = new QSortFilterProxyModel(this);
        tableProxy = new SmartSortProxy(this);
        tableProxy->setDynamicSortFilter(true);
        ui->tableData->setModel(tableProxy);
    } else {
        if (auto old = tableProxy->sourceModel())
            old->deleteLater();
    }
    tableProxy->setSourceModel(model);

    // >>> informe o record atual ao proxy, para ele saber os tipos declarados
    if (auto smart = dynamic_cast<SmartSortProxy*>(tableProxy)) {
        smart->setSqlRecord(currentRecord);
    }

    // reset de ordenação
    currentSortColumn = -1;
    currentSortState  = SortNone;
    resetSortToOriginalOrder();
    tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);

    tableView->resizeColumnsToContents();

    for (int col = 0; col < tableView->model()->columnCount(); ++col) {
        QVariant::Type type = currentRecord.field(col).type();

        int padding = 12;
        if (type == QVariant::Date || type == QVariant::DateTime) {
            padding = 22;
        }

        int currentWidth = tableView->columnWidth(col);
        tableView->setColumnWidth(col, currentWidth + padding);
    }

    if (comando == "SELECT" && hasId && !hasJoin && !hasSubquery) {
        rowsAffected = query.numRowsAffected();
        editEnabled = true;
        tableView->setEditTriggers(
            QAbstractItemView::SelectedClicked |
            QAbstractItemView::EditKeyPressed |
            QAbstractItemView::AnyKeyPressed |
            QAbstractItemView::DoubleClicked
            );

        CustomDelegate* delegate = new CustomDelegate(tableView, currentRecord);
        tableView->setItemDelegate(delegate);

        connect(delegate, &QAbstractItemDelegate::commitData, this, [=](QWidget* /*editor*/) {
            QModelIndex proxyIndex = ui->tableData->currentIndex();
            if (!proxyIndex.isValid() || !hasId || idPosition < 0)
                return;

            // mapeia do proxy para o source
            QModelIndex srcIdx = tableProxy->mapToSource(proxyIndex);

            const QString fieldName = currentRecord.fieldName(srcIdx.column());
            auto* srcModel = qobject_cast<QStandardItemModel*>(tableProxy->sourceModel());
            if (!srcModel) return;

            const QString newValue = srcIdx.model()->data(srcIdx, Qt::EditRole).toString();
            const QString oldValue = srcModel->item(srcIdx.row(), srcIdx.column())
                                         ? srcModel->item(srcIdx.row(), srcIdx.column())->data(Qt::UserRole).toString()
                                         : QString();

            if (newValue == oldValue) return; // nada mudou

            // empilha o comando; ele aplica e registra (DB ou cache), com undo/redo
            undoStack->push(new UpdateCellCommand(this,
                                                  QPersistentModelIndex(srcIdx),
                                                  idPosition,
                                                  fieldName,
                                                  oldValue,
                                                  newValue));
        });

        // connect(delegate, &QAbstractItemDelegate::commitData, this, [=](QWidget* editor) {
        //     QModelIndex proxyIndex = tableView->currentIndex();
        //     if (!proxyIndex.isValid() || !hasId || idPosition < 0)
        //         return;

        //     // mapear índice do proxy para o source
        //     QModelIndex index = tableProxy->mapToSource(proxyIndex);

        //     QString fieldName = currentRecord.fieldName(index.column());
        //     QString idValue = static_cast<QStandardItemModel*>(tableProxy->sourceModel())
        //                           ->item(index.row(), idPosition)->text();
        //     QString newValue = index.model()->data(index, Qt::EditRole).toString();
        //     QString oldValue = static_cast<QStandardItemModel*>(tableProxy->sourceModel())
        //                            ->item(index.row(), index.column())->data(Qt::UserRole).toString();
        //     if (oldValue != newValue) {
        //         if (!handleTableData_edit_trigger(idValue, fieldName, newValue)) {
        //             tableProxy->sourceModel()->blockSignals(true);
        //             tableProxy->sourceModel()->setData(index, oldValue, Qt::EditRole);
        //             tableProxy->sourceModel()->blockSignals(false);

        //             statusBar()->showMessage(
        //                 QString("Alteração REJEITADA em '%1'. Valor restaurado: %2")
        //                     .arg(fieldName, oldValue)
        //                 );
        //             qWarning() << "Alteração rejeitada. Valor restaurado.";
        //         }
        //         else {
        //             auto* item = static_cast<QStandardItemModel*>(tableProxy->sourceModel())
        //             ->item(index.row(), index.column());
        //             item->setData(newValue, Qt::UserRole);

        //             QFont font = item->font();
        //             font.setBold(true);
        //             item->setFont(font);
        //         }

        //     }
        // });

        connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex& current, const QModelIndex&) {
                    if (!currentRecord.isEmpty() && current.isValid()) {
                        // mapear índice selecionado do proxy para o source
                        QModelIndex sourceIdx = tableProxy->mapToSource(current);
                        QSqlField field = currentRecord.field(sourceIdx.column());
                        QVariant::Type type = field.type();
                        int fieldSize = field.length();

                        static const QMap<QVariant::Type, QString> sqlTypeMap = {
                            {QVariant::Int, "int"},
                            {QVariant::UInt, "unsigned int"},
                            {QVariant::LongLong, "bigint"},
                            {QVariant::ULongLong, "unsigned bigint"},
                            {QVariant::Double, "double"},
                            {QVariant::Bool, "boolean"},
                            {QVariant::Date, "date"},
                            {QVariant::Time, "time"},
                            {QVariant::DateTime, "datetime"},
                            {QVariant::String, "varchar"},
                            {QVariant::ByteArray, "blob"},
                            {QVariant::Char, "char"},
                            {QVariant::Invalid, "unknown"}
                        };

                        QString sqlTypeName = sqlTypeMap.value(type, QMetaType(type).name());
                        QString info;

                        if (fieldSize == 262140 && sqlTypeName == "varchar") {
                            info = QString("%1 • text").arg(field.name());
                        }
                        else {
                            info = QString("%1 • %2(%3)").arg(field.name(), sqlTypeName).arg(fieldSize);
                        }
                        statusBar()->showMessage(info);
                    }
                });

        statusMessage("Success • Rows: " + QString::number(rowsAffected));
    }
    else {
        editEnabled = false;
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void Sql::showChart()
{
    if (!ui->actionChart->isChecked())
    {
        return;
    }
    // Modelo atualmente mostrado na tabela (pode ser proxy)
    QAbstractItemModel *viewModel = ui->tableData->model();
    if (!viewModel) {
        statusMessage("Chart: sem dados (modelo nulo).");
        ui->actionChart->setChecked(false);
        ui->chartArea->setVisible(false);
        return;
    }

    // Se for proxy, usaremos o proxy diretamente para refletir filtro/ordem visíveis
    auto *proxy = qobject_cast<QSortFilterProxyModel*>(viewModel);

    const int rows = viewModel->rowCount();
    const int cols = viewModel->columnCount();

    if (rows == 0 || cols < 2) {
        statusMessage("Chart: é necessário pelo menos 1 linha e 2 colunas.");
        ui->actionChart->setChecked(false);
        ui->chartArea->setVisible(false);
        return;
    }

    // Helpers para classificação de tipos
    auto isNumericType = [](QVariant::Type t){
        return t == QVariant::Int || t == QVariant::UInt ||
               t == QVariant::LongLong || t == QVariant::ULongLong ||
               t == QVariant::Double;
    };
    auto isCategoryType = [](QVariant::Type t){
        return t == QVariant::String || t == QVariant::Char ||
               t == QVariant::Date || t == QVariant::DateTime || t == QVariant::Time;
    };

    // Encontrar uma coluna categórica (texto/data) para eixo X
    // Usa 'currentRecord' (tipos vindos do SELECT) e ignora a coluna "id"
    int categoryCol = -1;
    for (int c = 0; c < currentRecord.count() && c < cols; ++c) {
        const QString header = viewModel->headerData(c, Qt::Horizontal).toString().toLower();
        if (header == "id") continue;

        QVariant::Type t = currentRecord.field(c).type();
        if (isCategoryType(t)) {
            categoryCol = c;
            break;
        }
    }

    if (categoryCol == -1) {
        statusMessage("Chart: não há coluna categórica (texto/data) para o eixo X.");
        ui->actionChart->setChecked(false);
        ui->chartArea->setVisible(false);
        return;
    }

    // Selecionar colunas numéricas (séries)
    QVector<int> valueCols;
    for (int c = 0; c < currentRecord.count() && c < cols; ++c) {
        if (c == categoryCol) continue;
        QVariant::Type t = currentRecord.field(c).type();
        if (isNumericType(t)) valueCols.push_back(c);
    }

    if (valueCols.isEmpty()) {
        statusMessage("Chart: nenhuma coluna numérica encontrada para valores das barras.");
        ui->actionChart->setChecked(false);
        ui->chartArea->setVisible(false);
        return;
    }

    // Extrair categorias (rótulos do eixo X) a partir da coluna categórica,
    // usando o que está VISÍVEL (proxy/filtro/ordem). Usamos DisplayRole,
    // pois você já formatou datas no DisplayRole durante o preenchimento.
    QStringList categories;
    categories.reserve(rows);
    for (int r = 0; r < rows; ++r) {
        const QModelIndex idx = viewModel->index(r, categoryCol);
        categories << idx.data(Qt::DisplayRole).toString();
    }

    // Montar séries: cada coluna numérica vira um QBarSet
    auto *series = new QBarSeries();
    series->setLabelsVisible(false);

    double globalMax = 0.0;
    int colValidCount = 0;

    for (int c : valueCols) {
        const QString setName = viewModel->headerData(c, Qt::Horizontal).toString();
        auto *set = new QBarSet(setName);

        bool anyOk = false;
        for (int r = 0; r < rows; ++r) {
            const QModelIndex idx = viewModel->index(r, c);

            // Preferimos EditRole (valor bruto) para garantir conversão numérica estável
            bool ok = false;
            double v = idx.data(Qt::EditRole).toDouble(&ok);
            if (!ok) {
                // Tenta pelo DisplayRole como fallback (p.ex.: "123,45" em locais com vírgula)
                v = QLocale().toDouble(idx.data(Qt::DisplayRole).toString(), &ok);
            }
            if (!ok) v = 0.0; else anyOk = true;

            *set << v;
            if (v > globalMax) globalMax = v;
        }

        if (anyOk) {
            series->append(set);
            ++colValidCount;
        } else {
            // Nenhum valor numérico válido nessa coluna — descarta o set
            delete set;
        }
    }

    if (colValidCount == 0) {
        statusMessage("Chart: colunas numéricas não possuem valores válidos.");
        return;
    }

    // Construir o gráfico
    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Eixo X categórico
    auto *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // // Eixo Y numérico — ajusta range automaticamente com folga
    // auto *axisY = new QValueAxis();
    // double upper = (globalMax <= 0.0) ? 1.0 : globalMax * 1.1;
    // axisY->setRange(0.0, upper);
    // axisY->setLabelFormat("%g");
    // chart->addAxis(axisY, Qt::AlignLeft);
    // series->attachAxis(axisY);

    // Eixo Y numérico — apenas inteiros nas labels
    auto *axisY = new QValueAxis();

    // arredonda o topo para o próximo inteiro
    int upperInt = static_cast<int>(std::ceil(globalMax));
    if (upperInt <= 0) upperInt = 1;

    axisY->setRange(0, upperInt);
    axisY->setLabelFormat("%d");    // <<< força exibir só inteiros
    axisY->setMinorTickCount(0);    // opcional: sem marcas menores
    // axisY->setTickCount(qMin(11, upperInt + 1)); // opcional: controla qtde de marcas

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);


    // Título opcional (nome da tabela e/ou campo categórico)
    const QString catHeader = viewModel->headerData(categoryCol, Qt::Horizontal).toString();
    chart->setTitle(QString("%1 • %2")
                        .arg(databaseName, catHeader));

    // ChartView e colocação no QStackedWidget
    auto *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (auto old = ui->chartArea->currentWidget()) {
        ui->chartArea->removeWidget(old);
        old->deleteLater();
    }
    const int idx = ui->chartArea->addWidget(chartView);
    ui->chartArea->setCurrentIndex(idx);
    ui->chartArea->setVisible(true);

    statusMessage(QString("Chart: %1 categorias • %2 série(s) numérica(s).")
                      .arg(categories.size())
                      .arg(colValidCount));
}


void Sql::log(QString str)
{
    MainWindow* mainWin = qobject_cast<MainWindow*>(this->window());
    if (mainWin) {
        mainWin->log(sql_host, sql_schema, str);
    }
    else {
        qWarning() << "MainWindow not found from Sql::on_actionFavorites_triggered";
    }
}

bool Sql::handleTableData_edit_trigger(QString& id, QString& fieldName, QString& newValue)
{
    if (fieldName == "id")
    {
        return false;
    }
    newValue.remove('\'');
    QString queryStr = "UPDATE " + sql_table + " SET " + fieldName + " = '" + newValue + "' WHERE id = " + id;
    qDebug() << queryStr;
    if (!ui->actionAuto_commit->isChecked())
    {
        commitCache.append(queryStr);
        return true;
    }
    QSqlQuery query(dbMysqlLocal);
    if (!query.exec(queryStr)) {
        qWarning() << "Erro na query:" << query.lastError().text();
        return false;
    } else {
        log(queryStr);
    }
    return true;
}

void Sql::refresh_structure()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    if (dbMysqlLocal.open())
    {
        QSqlQuery query(dbMysqlLocal);
        QString queryStr = QString("USE %1;").arg(sql_schema);
        if (query.exec(queryStr)) {
            queryStr = "DESCRIBE " + sql_table;
            if (query.exec(queryStr)) {
                QStandardItemModel* model = new QStandardItemModel(this);
                model->setHorizontalHeaderLabels(QStringList() << "Field" << "Type" << "Null" << "Key" << "Default" << "Extra");

                int row = 0;
                while (query.next()) {
                    model->setItem(row, 0, new QStandardItem(query.value("Field").toString()));
                    model->setItem(row, 1, new QStandardItem(query.value("Type").toString()));
                    model->setItem(row, 2, new QStandardItem(query.value("Null").toString()));
                    model->setItem(row, 3, new QStandardItem(query.value("Key").toString()));
                    model->setItem(row, 4, new QStandardItem(query.value("Default").toString()));
                    model->setItem(row, 5, new QStandardItem(query.value("Extra").toString()));
                    ++row;
                }
            }
            else {
                qCritical() << "Erro ao obter estrutura:" << query.lastError().text();
            }
        }
    }
    else {
        qCritical() << "Failed to connect to the database";
    }

    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
}

void Sql::formatSqlText()
{
    QString texto = ui->textQuery->toPlainText();
    QStringList palavrasChave = {
        "FROM", "LEFT JOIN", "INNER JOIN", "RIGHT JOIN",
        "WHERE", "ORDER", "GROUP BY", "LIMIT"
    };

    for (const QString& palavra : palavrasChave) {
        QRegularExpression re(QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(palavra)), QRegularExpression::CaseInsensitiveOption);
        texto.replace(re, "\n" + palavra);
    }

    texto = texto.trimmed().replace(QRegularExpression("[ \\t]+"), " ");
    ui->textQuery->setPlainText(texto);
}

void Sql::on_actionRun_triggered()
{
    QString queryStr = ui->textQuery->textCursor().selectedText();
    if (queryTimer != "")
    {
        queryStr = queryTimer;
    }

    if (queryStr == "")
    {
        QTextCursor cursor = ui->textQuery->textCursor();
        int cursorPos = cursor.position();
        queryStr = extractCurrentQuery(ui->textQuery->toPlainText(), cursorPos);
    }
    if (queryStr != "")
    {
        queryStr = queryStr.trimmed();
        if (queryTimer == "")
        {
            queryStr = processQueryWithMacros(queryStr, this);
        }

        editEnabled = false;

        QApplication::setOverrideCursor(Qt::WaitCursor);
        QApplication::processEvents();

        QSqlQuery query(dbMysqlLocal);
        QApplication::processEvents();
        QString consulta = "USE " + databaseName + " ";
        if (!query.exec(consulta)) {
            qWarning() << "Erro ao selecionar banco de dados:" << query.lastError().text();
        }

        QString comando = queryStr.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).value(0).toUpper();

        if (comando == "SELECT")
        {
            QRegularExpression reFrom(R"(FROM\s+(\w+)(?:\s+(\w+))?)", QRegularExpression::CaseInsensitiveOption);
            if (auto match = reFrom.match(queryStr); match.hasMatch()) {
                tableName = match.captured(1).trimmed();
                tableAlias = match.captured(2).trimmed();
                QStringList sqlKeywords = {
                    "SELECT", "FROM", "WHERE", "JOIN", "LEFT", "RIGHT", "INNER", "OUTER", "ON",
                    "GROUP", "ORDER", "BY", "LIMIT", "HAVING", "AS", "AND", "OR", "NOT", "IN", "IS",
                    "NULL", "DESC", "ASC", "UNION", "ALL", "DISTINCT", "INSERT", "UPDATE", "DELETE"
                };
                if (!tableAlias.isEmpty() && sqlKeywords.contains(tableAlias.toUpper())) {
                    tableAlias.clear();
                }
            }

            QRegularExpression reSelect(R"(SELECT\s+(.*?)\s+FROM)", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
            if (auto match = reSelect.match(queryStr); match.hasMatch()) {
                QStringList rawFields = match.captured(1).split(',', Qt::SkipEmptyParts);
                selectFields = extractFieldsWithPrefix(rawFields, tableName, tableAlias);
            }

            QRegularExpression reWhere(R"(WHERE\s+(.*?)(ORDER\s+BY|LIMIT|$))", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
            if (auto match = reWhere.match(queryStr); match.hasMatch()) {
                QString whereClause = match.captured(1).trimmed();
                QRegularExpression reField(R"((\b\w+\.\w+|\b\w+)\s*(=|<>|!=|<|>|<=|>=|LIKE|IS|IN))", QRegularExpression::CaseInsensitiveOption);
                QStringList rawWhereFields;
                auto it = reField.globalMatch(whereClause);
                while (it.hasNext())
                    rawWhereFields << it.next().captured(1).trimmed();
                whereFields = extractFieldsWithPrefix(rawWhereFields, tableName, tableAlias);
            }

            QRegularExpression reOrder(R"(ORDER\s+BY\s+(.*?)(LIMIT|$))", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
            if (auto match = reOrder.match(queryStr); match.hasMatch()) {
                QStringList rawOrderFields = match.captured(1).split(',', Qt::SkipEmptyParts);
                for (QString& f : rawOrderFields) f = f.trimmed();
                orderByFields = extractFieldsWithPrefix(rawOrderFields, tableName, tableAlias);
            }

            hasJoin = queryStr.contains(QRegularExpression(R"(\bJOIN\b)", QRegularExpression::CaseInsensitiveOption));
            hasSubquery = queryStr.contains(QRegularExpression(R"(\(\s*SELECT\s+)", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption));
        }
        QApplication::processEvents();
        if (comando == "SELECT" || comando == "SHOW" || comando == "DESCRIBE" || comando == "EXPLAIN") {
            query2TableView(ui->tableData, queryStr, comando);
            showChart();
        }
        else {
            SafetyLinterHandler linter;
            QString sql = queryStr; // a mesma string que você vai executar

            if (!linter.handle(sql, this)) {
                statusMessage("Execução cancelada pelo usuário.");
                QApplication::restoreOverrideCursor();
                return; // aborta o run
            }
            // se chegou aqui, o usuário confirmou (ou não havia risco).

            editEnabled = false;
            ui->tableData->setEditTriggers(QAbstractItemView::NoEditTriggers);
            QSqlQuery query2(dbMysqlLocal);
            if (!query2.exec(queryStr)) {
                statusMessage("Command error: " + query2.lastError().text());
            }
            else {
                int linhasAfetadas = query2.numRowsAffected();
                statusMessage("Success! Line affected: " + QString::number(linhasAfetadas));
            }
            ui->tableData->setModel(nullptr);
        }
        QApplication::restoreOverrideCursor();
        QApplication::processEvents();
    }
    ui->textQuery->setFocus();
}

void Sql::on_actionFormat_triggered()
{
    formatSqlText();
}

void Sql::on_actionIncrease_triggered()
{
    setInterfaceSize(1);
}

void Sql::on_actionReduce_triggered()
{
    setInterfaceSize(-1);
}

void Sql::on_actionSave_triggered()
{
    updatePreferences();
    QString param = sql_host + "^" + sql_schema + "^" + sql_table;
    QString queryStr = ui->textQuery->textCursor().selectedText();
    if (queryStr == "")
    {
        queryStr = ui->textQuery->toPlainText();
    }
    setStringPreference(param, queryStr);
}

void Sql::statusMessage(QString msg)
{
    ui->statusbar->showMessage(QTime::currentTime().toString("HH:mm:ss") + " • " + msg);
}

void Sql::handleTimer_tick()
{
    if (button->isChecked())
    {
        int remaining = editTimes->text().toInt();
        if (remaining <= 1)
        {
            queryTimer = "";
            timer->stop();
            editTimes->setText("0");
            edit->setEnabled(true);
            button->setChecked(false);
        }
        else
        {
            if (queryTimer == "")
            {
                queryTimer = ui->textQuery->toPlainText();
                queryTimer = queryTimer.trimmed();
                queryTimer = processQueryWithMacros(queryTimer, this);
            }

            on_actionRun_triggered();
            if (ui->textQuery->styleSheet() == "QTextEdit {background-color: " + getRgbFromColorName(sql_color) + "}")
            {
                QString style = "QTextEdit {background-color: white}";
                ui->textQuery->setStyleSheet(style);
            }
            else {
                QString style = "QTextEdit {background-color: " + getRgbFromColorName(sql_color) + "}";
                ui->textQuery->setStyleSheet(style);
            }

            remaining--;
            editTimes->setText(QString::number(remaining));
        }
    }
}

void Sql::handleTableAppend_triggered()
{
    QString insert = QString("INSERT INTO %1 () VALUES ()").arg(sql_table);
    QSqlQuery query(dbMysqlLocal);
    if (!query.exec(insert)) {
        qWarning() << "Erro ao adicionar linha:" << query.lastError().text();
    }
    else {
        on_actionRun_triggered();

        // Depois que o modelo for aplicado, posiciona na última linha e coluna após o "id"
        QTimer::singleShot(0, this, [this]{
            QAbstractItemModel* model = ui->tableData->model();
            if (!model) return;

            const int lastRow = model->rowCount() - 1;
            if (lastRow < 0) return;

            const int cols = model->columnCount();

            // Usa a coluna após a do "id" (idPosition é atualizado em query2TableView)
            int targetCol = 0;
            if (idPosition >= 0 && idPosition + 1 < cols) {
                targetCol = idPosition + 1;
            } else if (cols > 1) {
                targetCol = 1;            // fallback: segunda coluna
            } else {
                targetCol = 0;            // fallback: primeira coluna
            }

            const QModelIndex idx = model->index(lastRow, targetCol);
            if (!idx.isValid()) return;

            ui->tableData->setCurrentIndex(idx);
            ui->tableData->scrollTo(idx, QAbstractItemView::PositionAtBottom);

            if (auto *sel = ui->tableData->selectionModel()) {
                sel->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }

            // Entra em edição se permitido
            if (editEnabled) ui->tableData->edit(idx);
        });
    }
}

void Sql::handleTableClone_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    int row = selection.first().row();
    QStringList fields, values;

    for (int col = 0; col < ui->tableData->model()->columnCount(); ++col) {
        if (ui->tableData->model()->headerData(col, Qt::Horizontal).toString().toLower() == "id")
            continue;
        QString field = ui->tableData->model()->headerData(col, Qt::Horizontal).toString();
        QString value = ui->tableData->model()->data(ui->tableData->model()->index(row, col)).toString();
        fields << "`" + field + "`";
        values << "'" + value.replace("'", "\\'") + "'";
    }

    QString insert = QString("INSERT INTO %1 (%2) VALUES (%3)")
                         .arg(sql_table)
                         .arg(fields.join(", "))
                         .arg(values.join(", "));

    QSqlQuery query(dbMysqlLocal);
    if (!query.exec(insert)) {
        qWarning() << "Erro ao clonar linha:" << query.lastError().text();
    }
    else {
        on_actionRun_triggered();

        // Depois que o modelo for aplicado, posiciona na última linha e coluna após o "id"
        QTimer::singleShot(0, this, [this]{
            QAbstractItemModel* model = ui->tableData->model();
            if (!model) return;

            const int lastRow = model->rowCount() - 1;
            if (lastRow < 0) return;

            const int cols = model->columnCount();

            // Usa a coluna após a do "id" (idPosition é atualizado em query2TableView)
            int targetCol = 0;
            if (idPosition >= 0 && idPosition + 1 < cols) {
                targetCol = idPosition + 1;
            } else if (cols > 1) {
                targetCol = 1;            // fallback: segunda coluna
            } else {
                targetCol = 0;            // fallback: primeira coluna
            }

            const QModelIndex idx = model->index(lastRow, targetCol);
            if (!idx.isValid()) return;

            ui->tableData->setCurrentIndex(idx);
            ui->tableData->scrollTo(idx, QAbstractItemView::PositionAtBottom);

            if (auto *sel = ui->tableData->selectionModel()) {
                sel->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }

            // Entra em edição se permitido
            if (editEnabled) ui->tableData->edit(idx);
        });

    }
}

void Sql::handleTableDelete_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    QSet<int> rows;
    for (const QModelIndex& index : selection)
        rows.insert(index.row());

    if (QMessageBox::question(this, "Confirmar exclusão",
                              QString("Deseja excluir %1 linha(s)?").arg(rows.size())) != QMessageBox::Yes)
        return;

    QSqlQuery query(dbMysqlLocal);
    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

    for (int row : sortedRows) {
        QString id = ui->tableData->model()->data(ui->tableData->model()->index(row, idPosition)).toString();
        QString del = QString("DELETE FROM %1 WHERE id = %2").arg(sql_table).arg(id);
        if (!query.exec(del)) {
            qWarning() << "Erro ao excluir linha id=" << id << "^" << query.lastError().text();
        }
        else {
            static_cast<QStandardItemModel*>(static_cast<QSortFilterProxyModel*>(ui->tableData->model())->sourceModel())->removeRow(
                static_cast<QSortFilterProxyModel*>(ui->tableData->model())->mapToSource(ui->tableData->model()->index(row, 0)).row()
                );
        }
    }
}

void Sql::handleTableCopyInsert_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    QAbstractItemModel* proxyModel = ui->tableData->model();
    auto* proxy = qobject_cast<QSortFilterProxyModel*>(proxyModel);
    QAbstractItemModel* model = proxy ? proxy->sourceModel() : proxyModel;

    // Remapear para índices do source para garantir consistência
    QMap<int, QMap<int, QString>> rowValues;
    for (const QModelIndex& proxyIdx : selection) {
        QModelIndex index = proxy ? proxy->mapToSource(proxyIdx) : proxyIdx;
        int row = index.row();
        int col = index.column();
        QString header = model->headerData(col, Qt::Horizontal).toString();
        Q_UNUSED(header);
        QString value = index.data().toString().replace("'", "\\'");
        rowValues[row][col] = "'" + value + "'";
    }

    QStringList insertCommands;
    for (auto it = rowValues.constBegin(); it != rowValues.constEnd(); ++it) {
        const QMap<int, QString>& colMap = it.value();
        QStringList columns;
        QStringList values;
        for (auto colIt = colMap.constBegin(); colIt != colMap.constEnd(); ++colIt) {
            QString columnName = model->headerData(colIt.key(), Qt::Horizontal).toString();
            columns << columnName;
            values << colIt.value();
        }

        QString query = QString("INSERT INTO %1 (%2) VALUES (%3);")
                            .arg(sql_table)
                            .arg(columns.join(", "))
                            .arg(values.join(", "));
        insertCommands << query;
    }

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(insertCommands.join("\n"));
}

void Sql::handleTableCopyUpdate_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    auto* proxy = qobject_cast<QSortFilterProxyModel*>(ui->tableData->model());
    QAbstractItemModel* model = proxy ? proxy->sourceModel() : ui->tableData->model();

    QMap<int, QMap<int, QString>> rowValues;
    int idColumn = -1;

    for (int col = 0; col < model->columnCount(); ++col) {
        QString header = model->headerData(col, Qt::Horizontal).toString();
        if (header.toLower() == "id") {
            idColumn = col;
            break;
        }
    }

    if (idColumn == -1) {
        qWarning() << "Coluna 'Id' não encontrada.";
        return;
    }

    for (const QModelIndex& proxyIdx : selection) {
        QModelIndex index = proxy ? proxy->mapToSource(proxyIdx) : proxyIdx;
        int row = index.row();
        int col = index.column();
        if (col == idColumn) continue;
        QString value = index.data().toString().replace("'", "\\'");
        rowValues[row][col] = "'" + value + "'";
    }

    QStringList updateCommands;
    for (auto it = rowValues.constBegin(); it != rowValues.constEnd(); ++it) {
        int row = it.key();
        const QMap<int, QString>& colMap = it.value();
        QModelIndex idIndex = model->index(row, idColumn);
        QString idValue = idIndex.data().toString();

        if (idValue.isEmpty()) {
            qWarning() << "Linha" << row << "não possui valor de ID.";
            continue;
        }

        QStringList assignments;
        for (auto colIt = colMap.constBegin(); colIt != colMap.constEnd(); ++colIt) {
            QString columnName = model->headerData(colIt.key(), Qt::Horizontal).toString();
            assignments << QString("%1 = %2").arg(columnName, colIt.value());
        }

        QString query = QString("UPDATE %1 SET %2 WHERE Id = %3;")
                            .arg(sql_table)
                            .arg(assignments.join(", "))
                            .arg(idValue);
        updateCommands << query;
    }

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(updateCommands.join("\n"));
}

void Sql::handleTableCopyCsv_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    // Ordenar seleção por linha/coluna
    std::sort(selection.begin(), selection.end(), [](const QModelIndex& a, const QModelIndex& b) {
        return a.row() == b.row() ? a.column() < b.column() : a.row() < b.row();
    });

    auto* proxy = qobject_cast<QSortFilterProxyModel*>(ui->tableData->model());
    QAbstractItemModel* model = proxy ? proxy->sourceModel() : ui->tableData->model();

    // Colunas selecionadas (source)
    QSet<int> selectedColumnsSet;
    for (const QModelIndex& proxyIdx : selection) {
        QModelIndex index = proxy ? proxy->mapToSource(proxyIdx) : proxyIdx;
        selectedColumnsSet.insert(index.column());
    }

    QList<int> selectedColumns = selectedColumnsSet.values();
    std::sort(selectedColumns.begin(), selectedColumns.end());

    QStringList headerRow;
    for (int column : selectedColumns) {
        QString header = model->headerData(column, Qt::Horizontal).toString();
        header.replace("\"", "\"\"");
        headerRow << "\"" + header + "\"";
    }

    QStringList csvText;
    csvText << headerRow.join(";");

    // Produzir CSV respeitando a ordem exibida (proxy) mas pegando dados remapeados
    int currentRow = -1;
    QStringList csvRow;
    for (const QModelIndex& proxyIdx : selection) {
        QModelIndex index = proxy ? proxy->mapToSource(proxyIdx) : proxyIdx;

        if (index.row() != currentRow) {
            if (currentRow != -1) csvText << csvRow.join(";");
            csvRow = QStringList(QString(selectedColumns.size(), QChar('\0')).split(QChar('\0')));
            currentRow = index.row();
        }

        int colIndex = selectedColumns.indexOf(index.column());
        if (colIndex != -1) {
            QVariant data = index.data();
            QString str = data.toString();
            str.replace("\"", "\"\"");

            QVariant::Type type = data.type();
            bool isNumeric = (type == QVariant::Int ||
                              type == QVariant::UInt ||
                              type == QVariant::LongLong ||
                              type == QVariant::ULongLong ||
                              type == QVariant::Double);

            bool needsQuotes = !isNumeric ||
                               str.contains(";") ||
                               str.contains("\n") ||
                               str.contains("\"") ||
                               str.startsWith("{") || str.startsWith("[");

            csvRow[colIndex] = needsQuotes ? "\"" + str + "\"" : str;
        }
    }
    if (!csvRow.isEmpty())
        csvText << csvRow.join(";");

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvText.join("\n"));
}

void Sql::handletableCRUDGfw_triggered()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText("<?php \n ?>");
}

void Sql::handletableCRUDLaravel_triggered()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText("<?php \n ?>");
}

void Sql::on_actionFavorites_triggered()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Favorite"));
    dialog.setMinimumWidth(300);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* label = new QLabel(tr("Name:"), &dialog);
    layout->addWidget(label);
    QLineEdit* lineEdit = new QLineEdit(&dialog);
    lineEdit->setText(favoriteName);
    layout->addWidget(lineEdit);
    lineEdit->setStyleSheet(
        "QLabel:last {"
        "   padding-right: 10px;"
        "   min-height: 30px;"
        "   min-width: 280px;"
        "}"
        "QPushButton {"
        "    padding: 4px;"
        "    margin: 20px;"
        "    min-height: 16px;"
        "}"
        );
    QCheckBox* sharedCheckbox = new QCheckBox(&dialog);
    sharedCheckbox->setText("Shared favorite");
    layout->addWidget(sharedCheckbox);

    QCheckBox* privateCheckbox = new QCheckBox(&dialog);
    privateCheckbox->setText("Private");
    layout->addWidget(privateCheckbox);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = lineEdit->text().trimmed();
        if (!name.isEmpty()) {
            QString value = ui->textQuery->toPlainText();
            // fav^host:schema:table:color:name
            if (sharedCheckbox->isChecked())
            {
                QString user = "";
                if (privateCheckbox->isChecked())
                {
                    user = getUserName();
                }
                setStringSharedPreference("fav^" + sql_host + "^" + sql_schema + "^" + sql_table + "^" + sql_color + "^" + name + "^" + user, value);
            }
            else {
                setStringPreference("fav^" + sql_host + "^" + sql_schema + "^" + sql_table + "^" + sql_color + "^" + name, value);
            }
            MainWindow* mainWin = qobject_cast<MainWindow*>(this->window());
            if (mainWin) {
                mainWin->refresh_favorites();
            }
            else {
                qWarning() << "MainWindow not found from Sql::on_actionFavorites_triggered";
            }
        }
    }
}

void Sql::on_actionMacros_triggered()
{
    MacroFormatDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString macro = dialog.resultMacro();
        QTextCursor cursor = ui->textQuery->textCursor();
        cursor.insertText(macro);
    }
}

void Sql::show_context_menu(const QPoint& pos)
{
    // --- atalho persistente (criado uma única vez) ---
    static QShortcut* newShortcut = nullptr;
    if (!newShortcut) {
        newShortcut = new QShortcut(QKeySequence::New, ui->tableData); // Ctrl+N / Cmd+N
        newShortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(newShortcut, &QShortcut::activated, this, [this]() {
            // usa a seleção atual quando o menu não está aberto
            QModelIndex current = ui->tableData->currentIndex();
            if (current.isValid())
                handleTableAppend_triggered();
        });
    }

    QMenu menu(this);

    QAction* tableAppend = menu.addAction("Append row");
    tableAppend->setShortcut(QKeySequence::New);                         // mostra "Ctrl+N" / "⌘N" no menu
    tableAppend->setShortcutContext(Qt::WidgetWithChildrenShortcut);     // funciona enquanto o menu está visível

    QAction* tableClone = menu.addAction("Clone row");
    QAction* tableDelete = menu.addAction("Delete row(s)");
    menu.addSeparator();
    QAction* tableCopyInsert = menu.addAction("Copy as INSERT");
    QAction* tableCopyUpdate = menu.addAction("Copy as UPDATE");
    QAction* tableCopyCsv = menu.addAction("Copy as CSV");
    menu.addSeparator();
    QAction* tableExportCSV = menu.addAction("Export as CSV");
    QAction* selectedAction = menu.exec(ui->tableData->viewport()->mapToGlobal(pos));




    if (selectedAction == tableClone) {
        handleTableClone_triggered();
    }
    else if (selectedAction == tableAppend) {
        handleTableAppend_triggered();
    }
    else if (selectedAction == tableDelete) {
        handleTableDelete_triggered();
    }
    else if (selectedAction == tableCopyInsert) {
        handleTableCopyInsert_triggered();
    }
    else if (selectedAction == tableCopyUpdate) {
        handleTableCopyUpdate_triggered();
    }
    else if (selectedAction == tableCopyCsv) {
        handleTableCopyCsv_triggered();
    }
}

// =========================
//  SUPORTE À ORDENAÇÃO
// =========================

void Sql::applySortState(int column)
{
    if (!tableProxy) return;
    auto* header = ui->tableData->horizontalHeader();

    switch (currentSortState) {
    case SortAsc:
        tableProxy->setSortRole(Qt::EditRole);
        tableProxy->sort(column, Qt::AscendingOrder);
        header->setSortIndicator(column, Qt::AscendingOrder);
        header->setSortIndicatorShown(true);
        break;
    case SortDesc:
        tableProxy->setSortRole(Qt::EditRole);
        tableProxy->sort(column, Qt::DescendingOrder);
        header->setSortIndicator(column, Qt::DescendingOrder);
        header->setSortIndicatorShown(true);
        break;
    case SortNone:
    default:
        resetSortToOriginalOrder();
        header->setSortIndicator(-1, Qt::AscendingOrder);
        header->setSortIndicatorShown(true);
        break;
    }
}

void Sql::resetSortToOriginalOrder()
{
    if (!tableProxy) return;
    tableProxy->setSortRole(OriginalRowRole);
    tableProxy->sort(0, Qt::AscendingOrder);
    tableProxy->setSortRole(Qt::EditRole);
}

void Sql::on_tableHeader_sectionClicked(int logicalIndex)
{
    if (currentSortColumn != logicalIndex) {
        currentSortColumn = logicalIndex;
        currentSortState = SortAsc;
    } else {
        if (currentSortState == SortAsc)
            currentSortState = SortDesc;
        else if (currentSortState == SortDesc)
            currentSortState = SortNone;
        else
            currentSortState = SortAsc;
    }

    applySortState(currentSortColumn);
    showChart();
}

void Sql::on_actionAuto_commit_triggered()
{
    setStringPreference("pref_autocommit", (ui->actionAuto_commit->isChecked() ? "1" : "0"));
}


void Sql::on_actionCommit_triggered()
{
    if (tableProxy && tableProxy->sourceModel()) {
        auto* model = static_cast<QStandardItemModel*>(tableProxy->sourceModel());

        for (int row = 0; row < model->rowCount(); ++row) {
            for (int col = 0; col < model->columnCount(); ++col) {
                QStandardItem* item = model->item(row, col);
                if (item) {
                    QFont font = item->font();
                    font.setBold(false);   // remove negrito
                    item->setFont(font);
                }
            }
        }
    }

    QSqlQuery query(dbMysqlLocal);
    for (int i = 0; i < commitCache.size(); ++i) {
        qDebug() << "Item" << i << ":" << commitCache[i];
        QString queryStr = commitCache[i];
        if (!query.exec(queryStr)) {
            qWarning() << "Erro na query:" << query.lastError().text();
            ui->statusbar->showMessage("Commit failed!");
        } else {
            log(queryStr);
        }
    }
    commitCache.clear();
    showChart();
    ui->statusbar->showMessage("Commit done.");
    return;
}


void Sql::on_actionChart_triggered()
{
    if (ui->actionChart->isChecked())
    {
        ui->chartArea->setVisible(true);
        showChart();
    } else {
        ui->chartArea->setVisible(false);
    }
}

