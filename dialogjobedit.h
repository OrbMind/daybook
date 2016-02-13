#ifndef DIALOGJOBEDIT_H
#define DIALOGJOBEDIT_H

#include <QDialog>
#include <QMessageBox>
#include <QtSql>
#include "maindef.h"

namespace Ui {
class DialogJobEdit;
}

namespace FullJobRecord {
    struct JobUserRecord{
        int idnJob;
        QString nameJob;
        bool bRecord;
        bool bRequest;
        int num;
    };
}

class DialogJobEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogJobEdit(QWidget *parent = 0);
    ~DialogJobEdit();

private:
    Ui::DialogJobEdit *ui;
    bool newJob;
    FullJobRecord::JobUserRecord currentJob;
    int currentUserRights;
    QSqlDatabase *db;

public slots:
    void recieveJobIdn(int jobIdn);
    void recieveUserPermissions(int userPermissions);
    void recieveDbSettings(QSqlDatabase *db);

private slots:
    void enableControls(bool enable);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    bool checkInput();
    void insertNewJob();
    void updateJob();

};

#endif // DIALOGJOBEDIT_H
