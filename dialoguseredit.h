#ifndef DIALOGUSEREDIT_H
#define DIALOGUSEREDIT_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include "maindef.h"

namespace Ui {
class DialogUserEdit;
}

class DialogUserEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUserEdit(QWidget *parent = 0);
    ~DialogUserEdit();

private:
    Ui::DialogUserEdit *ui;
    QSqlDatabase *db;
    bool newUser;
    int userIdn;
    int currentUserRights;
    int currentUserIdn;

public slots:
    void recieveDbSettings(QSqlDatabase *db);
    void recieveUserIdn(int userIdn);
    void recieveUserPermissions(int userPermissions);
    void recieveCurrentUserIdn(int userIdn);

private slots:
    void on_buttonBox_accepted();
    bool checkInput();
    void on_buttonBox_rejected();
    void refreshData();
    void enableControls(bool enable);
    bool checkTabNumber();
};

#endif // DIALOGUSEREDIT_H
