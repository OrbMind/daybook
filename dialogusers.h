#ifndef DIALOGUSERS_H
#define DIALOGUSERS_H

#include <QDialog>
#include <QtSql>
#include <QMenu>
#include <QMessageBox>
#include <dialoguseredit.h>

namespace Ui {
class DialogUsers;
}

class DialogUsers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUsers(QWidget *parent = 0);
    ~DialogUsers();

private slots:
    void on_pushButtonClose_clicked();
    void configTable();
    void refreshTable();
    void addNewUser();
    void editUser();
    void deleteUser();
    void restoreUser();
    void on_pushButtonAdd_clicked();
    void on_pushButtonEdit_clicked();
    void on_pushButtonDelete_clicked();
    void createContextTableMenu();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_pushButtonFind_clicked();
    void enableControls(bool enable);

    void on_tableWidget_doubleClicked(const QModelIndex &index);

public slots:
    void recieveDbSettings(QSqlDatabase *db);
    void recieveUserPermissions(int userPermissions);

signals:
    void sendDbSettings(QSqlDatabase *db);
    void sendUserIdn(int userIdn);
    void sendUserPermissions(int userPermissions);

private:
    Ui::DialogUsers *ui;
    QSqlDatabase *db;
    QMenu *contextTableMenu;
    int currentUserRights;
};

#endif // DIALOGUSERS_H
