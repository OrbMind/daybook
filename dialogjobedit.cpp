#include "dialogjobedit.h"
#include "ui_dialogjobedit.h"

DialogJobEdit::DialogJobEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogJobEdit)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Добавить новую должность");
    newJob = true;
}

DialogJobEdit::~DialogJobEdit()
{
    delete ui;
}

void DialogJobEdit::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
}

void DialogJobEdit::recieveJobIdn(int jobIdn)
{
    this->newJob = false;
    currentJob.idnJob = jobIdn;

    if (!db->open()) { QMessageBox::critical(this, tr("Database Error"), db->lastError().text()); return; }
    QSqlQuery query(*db);

    query.prepare("SELECT job,can_request,can_recorded,num FROM spr_job where idn=:idn;");
    query.bindValue(":idn",currentJob.idnJob);

    if ( !query.exec() || !query.next() )
    {
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        enableControls(false);
    }

    currentJob.nameJob = query.value("job").toString();
    currentJob.bRecord = query.value("can_recorded").toInt() == 1;
    currentJob.bRequest = query.value("can_request").toInt() == 1;
    currentJob.num = query.value("num").toInt();

    ui->lineEditJob->setText(currentJob.nameJob);
    ui->checkBoxRecord->setChecked(currentJob.bRecord);
    ui->checkBoxRequest->setChecked(currentJob.bRequest);
    ui->lineEditNum->setText(QString::number(currentJob.num));

    db->close();
}

void DialogJobEdit::recieveUserPermissions(int userPermissions)
{
    this->currentUserRights = userPermissions;
    enableControls(Act::userPermission(Act::editJob,userPermissions));

}

void DialogJobEdit::enableControls(bool enable)
{
    ui->lineEditJob->setEnabled(enable);
    ui->checkBoxRecord->setEnabled(enable);
    ui->checkBoxRequest->setEnabled(enable);
    ui->lineEditNum->setEnabled(enable);
}


void DialogJobEdit::on_buttonBox_accepted()
{
    if ( !Act::userPermission(Act::editJob,currentUserRights) ) return;
    if ( !checkInput() ) return;

    if ( newJob )
        insertNewJob();
    else
        updateJob();

    this->accept();
}

void DialogJobEdit::on_buttonBox_rejected()
{
    this->reject();
}

bool DialogJobEdit::checkInput()
{
    bool isok = false;

    if ( ui->lineEditJob->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(this, tr("Ошибка"), "Не заполнено поле наименование должности.");
        ui->lineEditJob->setFocus();
        return false;
    }

    ui->lineEditNum->text().trimmed().toInt(&isok);
    if ( !isok )
    {
        QMessageBox::warning(this, tr("Ошибка"), "В поле порядковый номер необходимо ввести число.");
        ui->lineEditNum->setFocus();
        return false;
    }

    return true;
}

void DialogJobEdit::insertNewJob()
{
    if (!db->open()) { QMessageBox::critical(this, tr("Database Error"), db->lastError().text()); }
    QSqlQuery query(*db);

    query.prepare("INSERT INTO spr_job (idn,job,deleted,can_request,can_recorded,num) VALUES (GEN_ID(gen_spr_job_idn, 1),:job,0,:can_request,:can_recorded,:num);");
    query.bindValue(":job",ui->lineEditJob->text().trimmed());
    query.bindValue(":can_request", QVariant((ui->checkBoxRequest->checkState() == Qt::Checked) ? 1 : 0).toInt() );
    query.bindValue(":can_recorded", QVariant(( ui->checkBoxRecord->checkState() == Qt::Checked) ? 1 : 0).toInt() );
    query.bindValue(":num",ui->lineEditNum->text().trimmed().toInt());

    if ( !query.exec() )
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    db->close();
}

void DialogJobEdit::updateJob()
{
    QString sq = "UPDATE spr_job SET ";
    bool addedField = false;

    if ( currentJob.nameJob != ui->lineEditJob->text().trimmed() )
    {
        sq = sq + "job=:job";
        addedField = true;
    }

    if ( currentJob.bRecord != (ui->checkBoxRecord->checkState() == Qt::Checked) )
    {
        if (addedField) sq = sq + ",";
        sq = sq + "can_recorded=:can_recorded";
        addedField = true;
    }

    if ( currentJob.bRequest != (ui->checkBoxRequest->checkState() == Qt::Checked ) )
    {
        if (addedField) sq = sq + ",";
        sq = sq + "can_request=:can_request";
        addedField = true;
    }

    if ( currentJob.num != ui->lineEditNum->text().trimmed().toInt() )
    {
        if (addedField) sq = sq + ",";
        sq = sq + "num=:num";
        addedField = true;
    }

    sq = sq + " WHERE idn=:idn;";

    if ( addedField )
    {
        if (!db->open()) { QMessageBox::critical(this, tr("Database Error"), db->lastError().text()); return; }

        QSqlQuery query(*db);
        query.prepare(sq);
        query.bindValue(":idn",currentJob.idnJob);

        if ( currentJob.nameJob != ui->lineEditJob->text().trimmed() )
            query.bindValue(":job",ui->lineEditJob->text().trimmed());

        if ( currentJob.bRecord != (ui->checkBoxRecord->checkState() == Qt::Checked) )
            query.bindValue(":can_recorded",QVariant((ui->checkBoxRecord->checkState() == Qt::Checked) ? 1 : 0).toInt());

        if ( currentJob.bRequest != (ui->checkBoxRequest->checkState() == Qt::Checked ) )
            query.bindValue(":can_request",QVariant((ui->checkBoxRequest->checkState() == Qt::Checked) ? 1 : 0).toInt());

        if ( currentJob.num != ui->lineEditNum->text().trimmed().toInt() )
            query.bindValue(":num", ui->lineEditNum->text().trimmed().toInt() );

        if ( !query.exec() )
            QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db->close();
    }

}
