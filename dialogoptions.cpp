#include "dialogoptions.h"
#include "ui_dialogoptions.h"

DialogOptions::DialogOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptions)
{
    ui->setupUi(this);
}

DialogOptions::~DialogOptions()
{
    delete ui;
}

void DialogOptions::recieveSettingsApp(QSettings **settings)
{
    this->settingsApp = *settings;
    readSettings();
}

void DialogOptions::readSettings()
{
    settingsApp->beginGroup("main");
    ui->checkBoxLastUser->setChecked(settingsApp->value("savelastuser",true).toBool());
    ui->checkBoxStoreFilter->setChecked(settingsApp->value("savefilter",false).toBool());
    ui->checkBoxWindowSizeNPos->setChecked(settingsApp->value("saveposition",false).toBool());
    settingsApp->endGroup();
}

void DialogOptions::writeSettings()
{
    settingsApp->beginGroup("main");
    settingsApp->setValue("savelastuser",ui->checkBoxLastUser->checkState() == Qt::Checked);
    settingsApp->setValue("savefilter",ui->checkBoxStoreFilter->checkState() == Qt::Checked);
    settingsApp->setValue("saveposition",ui->checkBoxWindowSizeNPos->checkState() == Qt::Checked);
    settingsApp->endGroup();
}

void DialogOptions::on_buttonBox_accepted()
{
    writeSettings();
}
