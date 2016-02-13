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
        QMessageBox::critical(this, tr("Database Error"), "Невозможно подключиться к базе данных.\n" + db->lastError().text());
        db->close();
        return false;
    }

    QSqlQuery query(*db);
    //проверка версии БД
    query.prepare("select svalue from appconfig where option='dbver';");
    if ( !query.exec() )
    {
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        return false;
    }
    if (!query.next())
    {
        QMessageBox::critical(this, tr("Query Error"), "Не возможно установить версию базы данных.\n Дальнейшая работа приложения невозможна.");
        return false;
    }

    if ( query.value("svalue").toString() != QString(ApplicationConfiguration::dbVersion) )
    {
        if ( query.value("svalue").toString() == QString(ApplicationConfiguration::olddbVersion) &&
                QMessageBox::critical(this, tr("Ошибка!"),
                              "Версия базы данных не совпадает.\nНеобходима: "
                              + QString(ApplicationConfiguration::dbVersion)
                              + "\nТекущая: " + query.value("svalue").toString()
                              + "\n Дальнейшая работа приложения невозможна."
                              + "\nОбновить базу данных до актуальной версии?",
                              QMessageBox::No,QMessageBox::Yes) == QMessageBox::Yes )
        {
            db->close();
            DialogUpdateDb* DialogUpdateDbWindow = new DialogUpdateDb(this);
            QString uName = db->userName();
            QString pass = db->password();
            connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), DialogUpdateDbWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
            emit sendDbSettings(this->db);
            this->setVisible(false);
            DialogUpdateDbWindow->exec();
            db->setUserName(uName);
            db->setPassword(pass);
            this->setVisible(true);
        }//if
        else
            QMessageBox::critical(this, tr("Ошибка!"),
                          "Версия базы данных не совпадает.\nНеобходима: "
                          + QString(ApplicationConfiguration::dbVersion)
                          + "\nТекущая: " + query.value("svalue").toString()
                          + "\n Дальнейшая работа приложения невозможна." );
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
        QMessageBox::critical(this, tr("Database Error"), "Невозможно подключиться к базе данных.\n" + db->lastError().text());
        return;
    }

    QSqlQuery query(*db);


    query.prepare("select idn from users where tab_number=:tab_number and upassword=:upassword and deleted=0;");
    query.bindValue(":tab_number",ui->lineEditName->text());
    query.bindValue(":upassword",ui->lineEditPassword->text());
    if ( !query.exec() )
    {
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        return;
    }
    if ( query.next() ){
        db->close();
        emit sendAuthorizedUserIdn(query.value("idn").toInt());
    }
    else
    {
        QMessageBox::warning(this, tr("Ошибка"), "Неправильное имя пользователя или пароль.");
        attemptNum++;
        if (attemptNum == 3)
        {
            //QMessageBox::warning(0, tr("Ошибка"), "Попытка №: " + QString::number(attemptNum));
            attemptNum = 0;
            //enableAllControls(false);
            //QEventLoop loop; QTimer::singleShot(600, &loop, SLOT(enableAllControls(true))); loop.exec();
        }
        db->close();
        return;
    }

    db->close();
    this->accept();

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

void DialogEnterSoft::recieveSettingsApp(QSettings **settings)
{
    this->settingsApp = *settings;
    bool saveLastUser;
    settingsApp->beginGroup("main");
    saveLastUser = settingsApp->value("savelastuser",true).toBool();
    settingsApp->endGroup();

    if ( saveLastUser )
    {
        settingsApp->beginGroup("main");
        ui->lineEditName->setText(settingsApp->value("lastuser","").toString());
        settingsApp->endGroup();
    }

    if ( ui->lineEditName->text().length() > 0 )
        ui->lineEditPassword->setFocus();
}
