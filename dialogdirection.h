#ifndef DIALOGDIRECTION_H
#define DIALOGDIRECTION_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QDesktopServices>
#include "maindef.h"

namespace Ui {
class DialogDirection;
}

class DialogDirection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDirection(QWidget *parent = 0);
    ~DialogDirection();

private slots:
    void on_buttonBox_rejected();
    void on_pushButtonSetSelectedUsers_clicked();
    void on_pushButtonSetAllUsers_clicked();
    void on_pushButtonRemoveAllUsers_clicked();
    void on_pushButtonRemoveSelectedUsers_clicked();
    void fillUsersList();
    void fillTodayDate();
    void fillNewNumber();
    void on_pushButtonChooseFilePath_clicked();
    void on_lineEditFilePath_textChanged(const QString &arg1);
    void on_buttonBox_accepted();
    bool CheckInput();
    void saveInitiatedUsers();
    void fillInitiatedUsers();
    void copyToListWidgetDirectionUsers();
    void createContextTableMenu();
    void on_listWidgetDirectionUsers_customContextMenuRequested(const QPoint &pos);
    void colourListWidget();
    void on_pushButtonFile_clicked();
    void enableControls(bool enable);
    void enableControlInitiate(bool enable);

private:
    Ui::DialogDirection *ui;
    QSqlDatabase *db;
    int directionIdn;
    int idnRequested;
    int idnRecorded;
    QMenu *contextTableMenu;
    int currentUserRights;

public slots:
    void recieveDbSettings(QSqlDatabase *db);
    void recieveDirectionIdn(int directionIdn);
    void recieveUserPermissions(int userPermissions);

};

#endif // DIALOGDIRECTION_H
