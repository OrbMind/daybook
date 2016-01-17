#ifndef DIALOGJOBEDIT_H
#define DIALOGJOBEDIT_H

#include <QDialog>
#include <QMessageBox>
#include "maindef.h"

namespace Ui {
class DialogJobEdit;
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
    QString jobName;
    int jobIdn;
    int currentUserRights;

public slots:
    void recieveJobName(QString jobName,int jobIdn);
    void recieveUserPermissions(int userPermissions);

private slots:
    void on_buttonBoxOkCancel_accepted();
    void enableControls(bool enable);

signals:
    void sendEditJob(QString jobName,int jobIdn,bool newJob);
};

#endif // DIALOGJOBEDIT_H
