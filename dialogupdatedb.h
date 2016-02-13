#ifndef DIALOGUPDATEDB_H
#define DIALOGUPDATEDB_H

#include <QDialog>
#include <QtSql>
#include <maindef.h>

namespace Ui {
class DialogUpdateDb;
}

class DialogUpdateDb : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUpdateDb(QWidget *parent = 0);
    ~DialogUpdateDb();

private slots:
    void on_pushButtonClose_clicked();
    void on_pushButtonUpdateDB_clicked();
    void addLog(QString text);
    bool checkConnection();
    bool checkVersion();
    bool updateV12V2();

private:
    Ui::DialogUpdateDb *ui;
    QSqlDatabase *db;

public slots:
    void recieveDbSettings(QSqlDatabase *db);
};

#endif // DIALOGUPDATEDB_H
