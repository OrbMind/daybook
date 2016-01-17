#ifndef DIALOGJOB_H
#define DIALOGJOB_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QMenu>
#include <dialogjobedit.h>

namespace Ui {
class DialogJob;
}

class DialogJob : public QDialog
{
    Q_OBJECT

public:
    explicit DialogJob(QWidget *parent = 0);
    ~DialogJob();

private slots:
    void on_pushButtonClose_clicked();
    void refreshTable();
    void configTable();
    void on_pushButtonFindClear_clicked();
    void addNewJob();
    void editJob();
    void deleteJob();
    void restoreJob();
    void on_pushButtonAddJob_clicked();
    void on_pushButtonEditJob_clicked();
    void on_pushButtonDeleteJob_clicked();
    void on_pushButtonFind_clicked();
    void createContextTableMenu();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void enableControls(bool enable);

    void on_tableWidget_doubleClicked(const QModelIndex &index);

private:
    Ui::DialogJob *ui;
    QSqlDatabase *db;
    QMenu *contextTableMenu;
    int currentUserRights;


public slots:
    void recieveDbSettings(QSqlDatabase *db);
    void recieveEditJob(QString jobName,int jobIdn,bool newJob);
    void recieveUserPermissions(int userPermissions);

signals:
    void sendJobName(QString jobName,int jobIdn);
    void sendUserPermissions(int userPermissions);
};

#endif // DIALOGJOB_H

