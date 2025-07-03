#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <QMainWindow>

namespace Ui {
class Structure;
}

class Structure : public QMainWindow
{
    Q_OBJECT

public:
    explicit Structure(QString &host, QString &schema, QString &table, QWidget *parent = nullptr);
    ~Structure();
    void refresh_structure();

private:
    Ui::Structure *ui;
};

#endif // STRUCTURE_H
