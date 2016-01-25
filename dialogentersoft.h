#ifndef DIALOGENTERSOFT_H
#define DIALOGENTERSOFT_H

#include <QDialog>
#include <QtSql>
#include "maindef.h"
#include "mainwindow.h"

namespace Ui {
class DialogEnterSoft;
}

class DialogEnterSoft : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEnterSoft(QWidget *parent = 0);
    //explicit DialogEnterSoft(QMainWindow *parent = 0);//QMainWindow
    ~DialogEnterSoft();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void enableAllControls(bool bEnable);
    bool checkDbAndVer();
    void on_commandLinkButtonGuest_clicked();
    
private:
    Ui::DialogEnterSoft *ui;
    QSqlDatabase *db;
    int attemptNum;
    QSettings *settingsApp;

public slots:
    void recieveDbSettings(QSqlDatabase *db);
    void recieveSettingsApp(QSettings **settings);

signals:
    void sendAuthorizedUserIdn(int userIdn);

};

#endif // DIALOGENTERSOFT_H
