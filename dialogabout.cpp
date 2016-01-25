#include "dialogabout.h"
#include "ui_dialogabout.h"

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);

    this->currentUserRights = UserRights::readonly;

    this->text = "Программа распространяется под лицензией GNU LGPL version 3\n"
            "http://www.gnu.org/copyleft/lesser.html\n"
            "\n"
            "Исходный код программы доступен по адресу:\n"
            "https://github.com/OrbMind/daybook\n"
            "\n"
            "При разработке программы использованы оригинальные и модифицированные иконки автора Feather:\n"
            "https://www.iconfinder.com/iconsets/feather\n"
            "Набор иконок распространяется по лицензии creative commons attribution 3.0:\n"
            "https://creativecommons.org/licenses/by/3.0/deed.ru"
            "\n"
            "\nВерсия: " + qApp->applicationVersion();;
    setText();
}

DialogAbout::~DialogAbout()
{
    delete ui;
}

void DialogAbout::on_pushButtonClose_clicked()
{
    this->accept();
}

void DialogAbout::recieveUserPermissions(int userPermissions)
{
    this->currentUserRights = userPermissions;
    setText();
}

void DialogAbout::setText()
{
    QString rText = this->text;

    QString adminInfo = "";

    if ( Act::userPermission(Act::showAdminInfo,currentUserRights) )
    {
        adminInfo = adminInfo + "\n"
                //"Сервер базы данных: " +
                //"Наименование базы данных: " +
                "\nИмя пользователя для подключения к БД: " + ApplicationConfiguration::userDBA +
                "\nПароль пользователя для подключения к БД: " + ApplicationConfiguration::passwordDBA;
    }

    ui->plainTextEditText->setPlainText( rText + adminInfo );
}

void DialogAbout::recieveSettingsApp(QSettings **settings)
{
    this->settingsApp = *settings;
    if ( Act::userPermission(Act::showAdminInfo,currentUserRights) )
    {
        ui->plainTextEditText->setPlainText(
                    ui->plainTextEditText->toPlainText() +
                    "\n\nФайл с настройками приложения: " + settingsApp->fileName());
    }
}
