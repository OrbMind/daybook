#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <dialogjob.h>
#include <dialogusers.h>
#include <dialogdirection.h>
#include <QMessageBox>
#include <QSettings>
#include <maindef.h>
#include <dialogabout.h>
#include <dialogentersoft.h>
#include <dialogoptions.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    bool enterSoft();
    void recieveAuthorizedUserIdn(int userIdn);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSettings *mainWindowSettings;
    QMenu *contextTableMenu;
    int currentUserIdn;
    int currentUserRights;
    QString currentUserLogin;
    QSettings *settingsApp;

private slots:
    void showJobSpr();
    void showUsersSpr();
    void showSettingsApp();
    void createMenuActions();
    void quitFromApp();
    void on_pushButtonMoreOptions_clicked();
    void on_pushButtonAdd_clicked();
    void refreshTable();
    void configTable();
    void on_pushButtonFind_clicked();
    void on_pushButtonEdit_clicked();
    void addNewDirection();
    void editDirection();
    void deleteDirection();
    void restoreDirection();
    bool configDB();
    void on_pushButtonDelete_clicked();
    void createContextTableMenu();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void showDialogAbout();
    void on_tableWidget_doubleClicked(const QModelIndex &index);
    void enableControls(bool enable);
    void on_comboBoxDate_currentIndexChanged(int index);
    void saveSettings();
    void readMainWindowPositionAndSize();
    void readMainWindowSettings();
    void readMainWindowFilterSettings();
    void closeEvent(QCloseEvent *);

signals:
    void sendDbSettings(QSqlDatabase *db);
    void sendUserPermissions(int userPermissions);
    void sendDirectionIdn(int directionIdn);
    void sendCurrentUserIdn(int userIdn);
    void sendSettingsApp(QSettings **settings);
};

#endif // MAINWINDOW_H
