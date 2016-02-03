#ifndef DIALOGCHANGEPASSWORD_H
#define DIALOGCHANGEPASSWORD_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>

namespace Ui {
class DialogChangePassword;
}

class DialogChangePassword : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChangePassword(QWidget *parent = 0);
    ~DialogChangePassword();

private:
    Ui::DialogChangePassword *ui;
    QSqlDatabase *db;
    int currentUserIdn;

public slots:
    void recieveDbSettings(QSqlDatabase *db);
    void recieveCurrentUserIdn(int userIdn);

private slots:
    void on_buttonBox_accepted();
    bool checkInput();
    void on_buttonBox_rejected();
};

#endif // DIALOGCHANGEPASSWORD_H
