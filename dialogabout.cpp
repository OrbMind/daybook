#include "dialogabout.h"
#include "ui_dialogabout.h"

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);
    ui->plainTextEditText->setPlainText
(
"Программа распространяется под лицензией GNU LGPL version 3\n"
"http://www.gnu.org/copyleft/lesser.html\n"
"\n"
"Исходный код программы доступен по адресу:\n"
"https://github.com/OrbMind/daybook\n"
"\n"
"При разработке программы использованы оригинальные и модифицированные иконки автора Feather:\n"
"https://www.iconfinder.com/iconsets/feather\n"
"Данный набор иконок распространяется по лицензии creative commons attribution 3.0:\n"
"https://creativecommons.org/licenses/by/3.0/deed.ru"
);

}

DialogAbout::~DialogAbout()
{
    delete ui;
}

void DialogAbout::on_pushButtonClose_clicked()
{
    this->accept();
}
