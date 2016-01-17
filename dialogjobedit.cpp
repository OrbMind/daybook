#include "dialogjobedit.h"
#include "ui_dialogjobedit.h"

DialogJobEdit::DialogJobEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogJobEdit)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Добавить новую должность");
    newJob = true;
    jobName = "";
    jobIdn = 0;
}

DialogJobEdit::~DialogJobEdit()
{
    delete ui;
}

void DialogJobEdit::recieveJobName(QString jobName,int jobIdn)
{
    this->jobName = jobName;
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Редактирование должности: " + this->jobName.trimmed());
    this->newJob = false;
    this->jobIdn = jobIdn;
    ui->lineEditJob->setText(this->jobName.trimmed());
}

void DialogJobEdit::on_buttonBoxOkCancel_accepted()
{
    if ( ui->lineEditJob->text().trimmed() == "")
    {
        QMessageBox::information(this, "Внимание!", "Наименование должности не может быть пустым.");
    } else if ( !newJob && ui->lineEditJob->text().trimmed() == jobName.trimmed() )
    {
        QMessageBox::information(this, "Внимание!", "Наименование должности не изменилось");
    } else
        emit sendEditJob(ui->lineEditJob->text(),jobIdn,newJob);
}

void DialogJobEdit::recieveUserPermissions(int userPermissions)
{
    this->currentUserRights = userPermissions;
    enableControls(Act::userPermission(Act::edit,userPermissions));

}

void DialogJobEdit::enableControls(bool enable)
{
    ui->lineEditJob->setEnabled(enable);
}

