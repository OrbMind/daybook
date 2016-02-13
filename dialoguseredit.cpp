#include "dialoguseredit.h"
#include "ui_dialoguseredit.h"

DialogUserEdit::DialogUserEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogUserEdit)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Создать нового пользователя");
    newUser = true;
    userIdn = -1;
    currentUserIdn = -1;
    ui->comboBoxPermissions->addItem("Пользователь",QVariant(UserRights::user));
    ui->comboBoxPermissions->addItem("Заполняющий",QVariant(UserRights::writer));
    ui->comboBoxPermissions->addItem("Администратор",QVariant(UserRights::admin));
}

DialogUserEdit::~DialogUserEdit()
{
    delete ui;
}

void DialogUserEdit::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
    refreshData();
}

void DialogUserEdit::recieveUserIdn(int userIdn)
{
    this->newUser = false;
    this->userIdn = userIdn;
    refreshData();
}

void DialogUserEdit::refreshData()
{
    if (!db->open()) { QMessageBox::critical(this, tr("Database Error"), db->lastError().text()); }
    QSqlQuery query(*db);
    int idnJob = -1;

    if ( !newUser )
    {
        query.prepare("select idn,idn_job,name,surname,patronymic,permissions,tab_number,upassword,users.surname || ' ' || LEFT(users.name,1) || '. ' || LEFT(users.patronymic,1) || '.' as initials from users where idn=:idn;");
        query.bindValue(":idn",userIdn);
        if ( !query.exec() )
            QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        query.next();
        ui->lineEditTabNumber->setText(query.value("tab_number").toString());
        ui->lineEditName->setText(query.value("name").toString());
        ui->lineEditSurname->setText(query.value("surname").toString());
        ui->lineEditPatronymic->setText(query.value("patronymic").toString());
        ui->lineEditPassword->setText(query.value("upassword").toString());
        idnJob = query.value("idn_job").toInt();
        ui->comboBoxPermissions->setCurrentIndex(ui->comboBoxPermissions->findData(QVariant(query.value("permissions").toInt())));
        this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Редактирование пользователя: " + query.value("initials").toString());
    }

    query.clear();
    if ( newUser )
        query.prepare("SELECT idn,job FROM spr_job WHERE DELETED=0;");
    else
    {
        query.prepare("SELECT idn,job FROM spr_job WHERE DELETED=0 or idn=:idn");
        query.bindValue(":idn",idnJob);
    }
    if ( !query.exec() )
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());

    ui->comboBoxJob->clear();
    while (query.next())
        ui->comboBoxJob->addItem(query.value(1).toString(),query.value(0).toInt());

    if ( !newUser )
        ui->comboBoxJob->setCurrentIndex(ui->comboBoxJob->findData(QVariant(idnJob)));
    db->close();

}

void DialogUserEdit::on_buttonBox_accepted()
{
    if ( !Act::userPermission(Act::editUser,currentUserRights) ) return;

    if ( !checkInput() ) return;

    if ( newUser)
    {
        if (!db->open()) { QMessageBox::critical(this, tr("Database Error"), db->lastError().text()); }
        QSqlQuery query(*db);
        query.prepare("INSERT INTO users (idn,idn_job,name,surname,patronymic,permissions,tab_number,upassword) VALUES (GEN_ID(gen_users_idn, 1),:idn_job,:name,:surname,:patronymic,:permissions,:tab_number,:upassword);");
        query.bindValue(":idn_job",ui->comboBoxJob->currentData(Qt::UserRole).toInt());
        query.bindValue(":name",ui->lineEditName->text().trimmed().remove(128,ui->lineEditName->text().length()));
        query.bindValue(":surname",ui->lineEditSurname->text().trimmed().remove(128,ui->lineEditSurname->text().length()));
        query.bindValue(":patronymic",ui->lineEditPatronymic->text().trimmed().remove(128,ui->lineEditPatronymic->text().length()));
        query.bindValue(":permissions",ui->comboBoxPermissions->currentData(Qt::UserRole).toInt());
        query.bindValue(":tab_number",ui->lineEditTabNumber->text().trimmed().remove(128,ui->lineEditTabNumber->text().length()));
        query.bindValue(":upassword",ui->lineEditPassword->text());
        if ( !query.exec() )
            QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db->close();
        this->accept();
    }else
    {
        if (!db->open()) { QMessageBox::critical(this, tr("Database Error"), db->lastError().text()); }
        QSqlQuery query(*db);

        //UPDATE spr_job SET deleted=1 WHERE idn=:idn
        query.prepare("UPDATE users SET idn_job=:idn_job,name=:name,surname=:surname,patronymic=:patronymic,tab_number=:tab_number,upassword=:upassword,permissions=:permissions WHERE idn=:idn");
        query.bindValue(":idn_job",ui->comboBoxJob->currentData(Qt::UserRole).toInt());
        query.bindValue(":name",ui->lineEditName->text().trimmed().remove(128,ui->lineEditName->text().length()));
        query.bindValue(":surname",ui->lineEditSurname->text().trimmed().remove(128,ui->lineEditSurname->text().length()));
        query.bindValue(":patronymic",ui->lineEditPatronymic->text().trimmed().remove(128,ui->lineEditPatronymic->text().length()));
        query.bindValue(":permissions",ui->comboBoxPermissions->currentData(Qt::UserRole).toInt());
        query.bindValue(":tab_number",ui->lineEditTabNumber->text().trimmed().remove(128,ui->lineEditTabNumber->text().length()));
        query.bindValue(":upassword",ui->lineEditPassword->text());
        query.bindValue(":idn",userIdn);

        if ( !query.exec() )
            QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db->close();
        this->accept();
    }
}

bool DialogUserEdit::checkInput()
{
    if ( ui->lineEditTabNumber->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(this, tr("Ошибка"), "Не заполнено поле табельный номер.");
        ui->lineEditTabNumber->setFocus();
        return false;
    }

    if ( !checkTabNumber() )
    {
        ui->lineEditTabNumber->setFocus();
        return false;
    }

    if ( ui->lineEditName->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(this, tr("Ошибка"), "Не заполнено поле имя.");
        ui->lineEditName->setFocus();
        return false;
    }

    if ( ui->lineEditSurname->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(this, tr("Ошибка"), "Не заполнено поле фамилия.");
        ui->lineEditSurname->setFocus();
        return false;
    }

    if ( ui->lineEditPatronymic->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(this, tr("Ошибка"), "Не заполнено поле отчество.");
        ui->lineEditPatronymic->setFocus();
        return false;
    }

    if ( ui->lineEditPassword->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(this, tr("Ошибка"), "Не заполнено поле пароль.");
        ui->lineEditPassword->setFocus();
        return false;
    }

    if ( ui->comboBoxJob->count() < 1 ||
         ui->comboBoxJob->currentIndex() == -1)
    {
        QMessageBox::warning(this, tr("Ошибка"), "Не указана должность.");
        ui->lineEditPassword->setFocus();
        return false;
    }

    return true;
}

void DialogUserEdit::on_buttonBox_rejected()
{
    this->reject();
}

void DialogUserEdit::recieveUserPermissions(int userPermissions)
{
    this->currentUserRights = userPermissions;
    enableControls(Act::userPermission(Act::editUser,userPermissions));
}

void DialogUserEdit::enableControls(bool enable)
{
    ui->lineEditName->setEnabled(enable);
    ui->comboBoxJob->setEnabled(enable);
    ui->comboBoxPermissions->setEnabled(enable);
    ui->lineEditPassword->setEnabled(enable);
    ui->lineEditPatronymic->setEnabled(enable);
    ui->lineEditSurname->setEnabled(enable);
    ui->lineEditTabNumber->setEnabled(enable);

    if ( !enable )
        ui->lineEditTabNumber->setEchoMode(QLineEdit::Password);
}

bool DialogUserEdit::checkTabNumber()
{
    bool result = false;
    if (!db->open()) { QMessageBox::critical(this, tr("Database Error"), db->lastError().text()); }
    QSqlQuery query(*db);



    query.prepare("select users.idn, users.surname || ' ' || LEFT(users.name,1) || '. ' || LEFT(users.patronymic,1) || '.' as initials,spr_job.job,users.deleted from users,spr_job"
                  " where spr_job.idn = users.idn_job and users.tab_number=:tab_number;");
    query.bindValue(":tab_number",ui->lineEditTabNumber->text().trimmed().remove(128,ui->lineEditTabNumber->text().length()));
    if ( !query.exec() )
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    if ( query.next() && query.value("idn").toInt() != userIdn )
    {
        QMessageBox::warning(this, tr("Ошибка"), "Пользователь с таким табельным номером уже существует: " + query.value("initials").toString());
        result = false;
    }
    else
    {
        result = true;
    }
    return result;
}

void DialogUserEdit::recieveCurrentUserIdn(int userIdn)
{
    this->currentUserIdn = userIdn;
}
