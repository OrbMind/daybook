#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H

#include <QDialog>
#include <QSettings>
#include <maindef.h>

namespace Ui {
class DialogAbout;
}

class DialogAbout : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAbout(QWidget *parent = 0);
    ~DialogAbout();

private slots:
    void on_pushButtonClose_clicked();
    void setText();

private:
    Ui::DialogAbout *ui;
    int currentUserRights;
    QString text;
    QSettings *settingsApp;

public slots:
    void recieveUserPermissions(int userPermissions);
    void recieveSettingsApp(QSettings **settings);
};

#endif // DIALOGABOUT_H
