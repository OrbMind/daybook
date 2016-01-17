#include "dialogentersoft.h"
#include "ui_dialogentersoft.h"

DialogEnterSoft::DialogEnterSoft(QWidget *parent) :
//DialogEnterSoft::DialogEnterSoft(QMainWindow *parent) ://QMainWindow
    QDialog(parent),
    ui(new Ui::DialogEnterSoft)
{
    ui->setupUi(this);
    this->attemptNum = 0;
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": вход");
    connect(this,SIGNAL(sendAuthorizedUserIdn(int)),parent, SLOT(recieveAuthorizedUserIdn(int)));
}

DialogEnterSoft::~DialogEnterSoft()
{
    delete ui;
}

bool DialogEnterSoft::checkDbAndVer()
{
    //try to open database
    if (!db->open())
    {
        QMessageBox::critical(0, tr("Database Error"), "Невозможно подключиться к базе данных.\n" + db->lastError().text());
        return false;
    }

    QSqlQuery query(*db);
    //проверка версии БД
    query.prepare("select svalue from appconfig where option='dbver';");
    if ( !query.exec() )
    {
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        return false;
    }
    if (!query.next())
    {
        QMessageBox::critical(0, tr("Query Error"), "Не возможно установить версию базы данных.\n Дальнейшая работа приложения невозможна.");
        return false;
    }

    if ( query.value("svalue").toString() != QString(ApplicationConfiguration::dbVersion) )
    {
        QMessageBox::critical(0, tr("Query Error"), "Версия базы данных не совпадает.\nНеобходима: " + QString(ApplicationConfiguration::dbVersion) + "\nТекущая: " + query.value("svalue").toString() + "\n Дальнейшая работа приложения невозможна.");
        return false;
    }
    db->close();
    return true;
}

void DialogEnterSoft::on_buttonBox_accepted()
{
    if ( ! checkDbAndVer() ) return;

    //try to open database
    if (!db->open())
    {
        QMessageBox::critical(0, tr("Database Error"), "Невозможно подключиться к базе данных.\n" + db->lastError().text());
        return;
    }

    QSqlQuery query(*db);


    //QMessageBox::warning(0, tr("Ошибка"), "Проверка версии БД!");
    query.prepare("select idn from users where tab_number=:tab_number and upassword=:upassword;");
    query.bindValue(":tab_number",ui->lineEditName->text());
    query.bindValue(":upassword",ui->lineEditPassword->text());
    if ( !query.exec() )
    {
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        return;
    }
    if ( query.next() ){
        db->close();
        emit sendAuthorizedUserIdn(query.value("idn").toInt());
    }
    else
    {
        QMessageBox::warning(0, tr("Ошибка"), "Неправильное имя пользователя или пароль.");
        attemptNum++;
        if (attemptNum == 3)
        {
            //QMessageBox::warning(0, tr("Ошибка"), "Попытка №: " + QString::number(attemptNum));
            attemptNum = 0;
            //enableAllControls(false);
            //QEventLoop loop; QTimer::singleShot(600, &loop, SLOT(enableAllControls(true))); loop.exec();
        }
        return;
    }

    this->accept();
    db->close();
}

void DialogEnterSoft::on_buttonBox_rejected()
{
    this->reject();
}

void DialogEnterSoft::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
}

void DialogEnterSoft::enableAllControls(bool bEnable)
{
    ui->lineEditName->setEnabled(bEnable);
    ui->lineEditPassword->setEnabled(bEnable);
    ui->buttonBox->setEnabled(bEnable);
}

void DialogEnterSoft::on_commandLinkButtonGuest_clicked()
{
    if ( ! checkDbAndVer() ) return;
    emit sendAuthorizedUserIdn(ApplicationConfiguration::idnGuest);
    this->accept();
}
