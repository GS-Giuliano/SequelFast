#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void refresh_tables(QString selectedHost, QString selectedSchema);

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_listViewConns_clicked(const QModelIndex &index);

    void on_actionQuit_triggered();

    void on_buttonNewConns_clicked();

    void on_actionNew_connection_triggered();

    void on_buttonFilterSchemas_triggered(QAction *arg1);

    void on_listViewSchemas_clicked(const QModelIndex &index);

    void on_buttonUpdateSchemas_clicked();

    void on_buttonUpdateTables_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
