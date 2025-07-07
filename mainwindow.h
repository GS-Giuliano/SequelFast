#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStyledItemDelegate>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    bool host_connect(QString selectedHost);

    void refresh_connections();
    void refresh_schemas(QString selectedHost);
    void refresh_schema(QString selectedSchema);
    void refresh_tables(QString selectedHost);
    void startSSH(QString &selectedHost);
    void endSSH(QString &selectedHost);

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_buttonNewConns_clicked();
    void on_buttonFilterSchemas_clicked();
    void on_buttonEditConns_clicked();
    void on_buttonEditTables_clicked();
    void on_buttonEditSchemas_clicked();
    void on_buttonFilterTables_clicked();
    void on_buttonUpdateSchemas_clicked();
    void on_buttonUpdateTables_clicked();
    void on_toolBoxLeft_currentChanged(int index);

    void on_actionQuit_triggered();
    void on_actionNew_connection_triggered();
    void on_actionTile_triggered();
    void on_actionCascade_triggered();

    void on_listViewConns_clicked(const QModelIndex &index);
    void on_listViewSchemas_clicked(const QModelIndex &index);
    void on_listViewTables_clicked(const QModelIndex &index);

    void listViewConns_open(const QModelIndex &index);
    void listViewConns_edit(const QModelIndex &index);
    void listViewConns_clone(const QModelIndex &index);
    void listViewConns_remove(const QModelIndex &index);
    void mostrarMenuContextoConns(const QPoint &pos);

    void mostrarMenuContextoSchemas(const QPoint &pos);
    void mostrarMenuContextoTables(const QPoint &pos);


private:
    Ui::MainWindow *ui;

    QAction* action_db_options;
};

class ColoredItemDelegate : public QStyledItemDelegate {
public:
    ColoredItemDelegate(QObject *parent = nullptr, const QString &nomeDestacado = "")
        : QStyledItemDelegate(parent), destaque(nomeDestacado) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        if (index.data().toString() == destaque) {
            opt.backgroundBrush = QBrush(Qt::yellow);
            opt.palette.setColor(QPalette::Text, Qt::blue);
        }

        QStyledItemDelegate::paint(painter, opt, index);
    }

private:
    QString destaque;
};

#endif // MAINWINDOW_H
