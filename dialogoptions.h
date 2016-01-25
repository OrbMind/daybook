#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class DialogOptions;
}

class DialogOptions : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptions(QWidget *parent = 0);
    ~DialogOptions();

private:
    Ui::DialogOptions *ui;
    QSettings *settingsApp;

public slots:
    void recieveSettingsApp(QSettings **settings);

private slots:
    void readSettings();
    void writeSettings();
    void on_buttonBox_accepted();
};

#endif // DIALOGOPTIONS_H
