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

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_listViewConns_clicked(const QModelIndex &index);

    void on_actionQuit_triggered();

    void on_buttonNewConns_clicked();

    void on_actionNew_connection_triggered();

    void on_listViewSchemas_clicked(const QModelIndex &index);

    void on_buttonUpdateSchemas_clicked();

    void on_buttonUpdateTables_clicked();

    void on_listViewTables_clicked(const QModelIndex &index);

    void on_toolBoxLeft_currentChanged(int index);

private:
    Ui::MainWindow *ui;
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
