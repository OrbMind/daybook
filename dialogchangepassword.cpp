#include "dialogchangepassword.h"
#include "ui_dialogchangepassword.h"

DialogChangePassword::DialogChangePassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogChangePassword)
{
    ui->setupUi(this);
}

DialogChangePassword::~DialogChangePassword()
{
    delete ui;
}

void DialogChangePassword::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
}

void DialogChangePassword::recieveCurrentUserIdn(int userIdn)
{
    this->currentUserIdn = userIdn;
}

void DialogChangePassword::on_buttonBox_accepted()
{
    if ( checkInput() )
    {
        if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); return; }
        QSqlQuery query(*db);
        query.prepare("update users set upassword=:upassword where idn=:idn;");
        query.bindValue(":upassword",ui->lineEditNewPassword->text());
        query.bindValue(":idn",currentUserIdn);

        if ( !query.exec() )
            QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());

        db->close();
        this->accept();
    }
}

bool DialogChangePassword::checkInput()
{

    if ( ui->lineEditOldPassword->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(0, tr("Ошибка"), "Не заполнено поле текущий пароль.");
        ui->lineEditOldPassword->setFocus();
        return false;
    }

    if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); return false; }
    QSqlQuery query(*db);
    query.prepare("select idn from users where upassword=:upassword and idn=:idn;");
    query.bindValue(":upassword",ui->lineEditOldPassword->text());
    query.bindValue(":idn",currentUserIdn);
    if ( !query.exec() )
    {
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db->close();
        return false;
    }
    if ( !query.next() || query.value("idn").toInt() != currentUserIdn )
    {
        QMessageBox::warning(0, tr("Ошибка"), "Пароль пользователя не правильный.");
        db->close();
        ui->lineEditOldPassword->setFocus();
        return false;
    }
    query.clear();
    db->close();

    if ( ui->lineEditNewPassword->text().length() == 0 )
    {
        QMessageBox::warning(0, tr("Ошибка"), "Не заполнено поле новый пароль.");
        ui->lineEditNewPassword->setFocus();
        return false;
    }

    if ( ui->lineEditConfirmPassword->text().length() == 0 )
    {
        QMessageBox::warning(0, tr("Ошибка"), "Не заполнено поле подтверждение пароля.");
        ui->lineEditConfirmPassword->setFocus();
        return false;
    }

    if ( QString::compare( ui->lineEditNewPassword->text(),
                           ui->lineEditConfirmPassword->text(),
                           Qt::CaseSensitive) != 0 )
    {
        QMessageBox::warning(0, tr("Ошибка"), "Новый пароль и подтверждение не совпадают.");
        ui->lineEditNewPassword->setFocus();
        return false;
    }

    return true;
}

void DialogChangePassword::on_buttonBox_rejected()
{
    this->reject();
}
