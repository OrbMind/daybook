#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWindowSettings = NULL;
    contextTableMenu = NULL;
    settingsApp = NULL;
    currentUserRights = UserRights::readonly;

    this->settingsApp = new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
            QDir::separator() +
            QCoreApplication::applicationName() + ".ini",
            QSettings::IniFormat);

    this->setWindowTitle(QString(ApplicationConfiguration::fullNameApplication));


    createMenuActions();

    ui->widgetExtendedOptions->setVisible(true);
    ui->pushButtonMoreOptions->setIcon(QIcon(":/images/icon/less.png"));

    if( ! configDB() ) exit(1);

    configTable();

    contextTableMenu = new QMenu(this);
    createContextTableMenu();

    ui->comboBoxDate->addItem("Указанные даты");
    ui->comboBoxDate->addItem("За эту неделю");
    ui->comboBoxDate->addItem("За этот месяц");
    ui->comboBoxDate->addItem("За этот год");

#ifdef Q_OS_WIN32
    ui->actionCreateShortcut->setEnabled(true);
#else
    ui->actionCreateShortcut->setEnabled(false);
#endif
}

void MainWindow::prepareWindow()
{
    on_comboBoxDate_currentIndexChanged(3);
    on_comboBoxDate_currentIndexChanged(0);

    readMainWindowSettings();

    ui->pushButtonFind->click();
}

bool MainWindow::enterSoft()
{
    DialogEnterSoft* dialogEnterSoftWindow = new DialogEnterSoft(this);
    dialogEnterSoftWindow->setModal(true);
    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogEnterSoftWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(&this->db);
    connect(this, SIGNAL(sendSettingsApp(QSettings**)),dialogEnterSoftWindow, SLOT(recieveSettingsApp(QSettings**)));
    emit sendSettingsApp(&settingsApp);
    bool Result = dialogEnterSoftWindow->exec() == QDialog::Accepted;
    //dialogEnterSoftWindow->~DialogEnterSoft();
    //dialogEnterSoftWindow = NULL;
    return Result;
}

bool MainWindow::configDB()
{
    QDir appStartDir(    QDir::toNativeSeparators( qApp->applicationDirPath() ) );
    QFile mainConfigFile(    QDir::toNativeSeparators(
                         appStartDir.absolutePath() +
                         QDir::separator() +
                         qApp->applicationName() +
                         ".conf" ) );
    /*if( !mainConfigFile.exists() )
    {
        QMessageBox::critical(this, tr("Main config error"), "Файл конфигурации приложения не найден:\n"+mainConfigFile.fileName()+
                              "\n\nМинимальный файл конфигурации имеет вид:\n"+
                              "[main]\n"+
                              "hostname=localhost\n"+
                              "databasename=daybook");
        return false;
    }*/
    mainWindowSettings = new QSettings( mainConfigFile.fileName(),
                                 QSettings::IniFormat);

    db = QSqlDatabase::addDatabase("QIBASE");

    mainWindowSettings->beginGroup("main");
    db.setHostName(mainWindowSettings->value("hostname",ApplicationConfiguration::defaultHost).toString());
    db.setDatabaseName(mainWindowSettings->value("databasename",ApplicationConfiguration::defaultDbName).toString());
    mainWindowSettings->endGroup();

    /*int size = mainWindowSettings->beginReadArray("connectoptions");
    for(int i=0; i<size; ++i)
    {
        mainWindowSettings->setArrayIndex(i);
        mainWindowSettings->value();
        //ui->lvFileList->addItem(settingsApp->value("File").toString());
    }
    settingsApp->endArray();*/
    db.setConnectOptions(ApplicationConfiguration::connectionOptions);


    db.setUserName(ApplicationConfiguration::userDBA);
    db.setPassword(ApplicationConfiguration::passwordDBA);

        return true;
}

MainWindow::~MainWindow()
{
    if ( mainWindowSettings != NULL )
    {
        mainWindowSettings->~QSettings();
        mainWindowSettings = NULL;
    }
    if (contextTableMenu != NULL)
    {
        contextTableMenu->~QMenu();
        contextTableMenu = NULL;
    }
    if ( settingsApp != NULL )
    {
        settingsApp->~QSettings();
        settingsApp = NULL;
    }
    delete ui;
}

void MainWindow::createMenuActions()
{
    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(quitFromApp()));
    connect(ui->actionJob,SIGNAL(triggered()),this,SLOT(showJobSpr()));
    connect(ui->actionUsers,SIGNAL(triggered()),this,SLOT(showUsersSpr()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(showDialogAbout()));
    connect(ui->actionMainSettings,SIGNAL(triggered()),this,SLOT(showSettingsApp()));
    connect(ui->actionChangePassword,SIGNAL(triggered(bool)),this,SLOT(showDialogChangePassword()));
    connect(ui->actionCreateShortcut,SIGNAL(triggered(bool)),this,SLOT(makeCreateShortcut()));
}

void MainWindow::makeCreateShortcut()
{
    QDir::setCurrent(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    QFile::link(qApp->applicationFilePath(),QString(ApplicationConfiguration::briefNameApplication) + ".lnk");
}

void MainWindow::showDialogAbout()
{
    DialogAbout* dialogAboutWindow = new DialogAbout;
    dialogAboutWindow->setModal(true);

    connect(this,SIGNAL(sendUserPermissions(int)),dialogAboutWindow, SLOT(recieveUserPermissions(int)));
    emit sendUserPermissions(this->currentUserRights);

    connect(this,SIGNAL(sendSettingsApp(QSettings**)),dialogAboutWindow, SLOT(recieveSettingsApp(QSettings**)));
    emit sendSettingsApp(&settingsApp);

    dialogAboutWindow->exec();

    dialogAboutWindow->~DialogAbout();
    dialogAboutWindow = NULL;
}

void MainWindow::showDialogChangePassword()
{
    DialogChangePassword* dialogChangePasswordWindow = new DialogChangePassword;
    dialogChangePasswordWindow->setModal(true);

    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogChangePasswordWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(&this->db);

    connect(this,SIGNAL(sendCurrentUserIdn(int)),dialogChangePasswordWindow,SLOT(recieveCurrentUserIdn(int)));
    emit sendCurrentUserIdn(currentUserIdn);

    dialogChangePasswordWindow->exec();
    dialogChangePasswordWindow->~DialogChangePassword();
    dialogChangePasswordWindow = NULL;
}

void MainWindow::showJobSpr()
{
    DialogJob* dialogJobWindow = new DialogJob;
    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogJobWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(&this->db);

    connect(this,SIGNAL(sendUserPermissions(int)),dialogJobWindow, SLOT(recieveUserPermissions(int)));
    emit sendUserPermissions(this->currentUserRights);

    this->hide();
    dialogJobWindow->exec();
    this->show();

    dialogJobWindow->~DialogJob();
    dialogJobWindow = NULL;
}

void MainWindow::showUsersSpr()
{
    DialogUsers* dialogUsersWindow = new DialogUsers;
    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogUsersWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(&this->db);

    connect(this,SIGNAL(sendUserPermissions(int)),dialogUsersWindow, SLOT(recieveUserPermissions(int)));
    emit sendUserPermissions(this->currentUserRights);

    connect(this,SIGNAL(sendCurrentUserIdn(int)),dialogUsersWindow,SLOT(recieveCurrentUserIdn(int)));
    emit sendCurrentUserIdn(currentUserIdn);

    this->hide();
    dialogUsersWindow->exec();
    this->show();

    dialogUsersWindow->~DialogUsers();
    dialogUsersWindow = NULL;
}

void MainWindow::quitFromApp()
{
    qApp->quit();
}

void MainWindow::on_pushButtonMoreOptions_clicked()
{
    ui->widgetExtendedOptions->setVisible(!ui->widgetExtendedOptions->isVisible());
    if ( ui->widgetExtendedOptions->isVisible() )
        ui->pushButtonMoreOptions->setIcon(QIcon(":/images/icon/less.png"));
    else
        ui->pushButtonMoreOptions->setIcon(QIcon(":/images/icon/more.png"));
}

void MainWindow::on_pushButtonAdd_clicked()
{
    addNewDirection();
}

void MainWindow::refreshTable()
{
    int selectedIdn = -1;
    if ( ui->tableWidget->selectedItems().count() > 0)
        selectedIdn = QVariant(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(DataRole::idn)).toInt();
    ui->tableWidget->setSortingEnabled(false);
    //clear table
    int n = ui->tableWidget->rowCount();
    for( int i =0; i < n; i++ ) ui->tableWidget->removeRow(0);

    if (!db.open()) { QMessageBox::critical(this, tr("Database Error"), "last error:" + db.lastError().text()); }
    QSqlQuery query(db);

    //query.prepare("select idn,num,ddate,subject,deleted from directions;");
    QString sq="";
    sq = "select directions.idn,directions.num,"
    " CAST(lpad(EXTRACT(DAY FROM directions.ddate),2,'0') AS varchar(2))||'.'|| CAST(lpad(EXTRACT(MONTH FROM directions.ddate),2,'0') AS varchar(2))||'.'|| EXTRACT(YEAR FROM directions.ddate) AS fdate"
    " ,directions.subject,directions.deleted from directions";
    if ( ui->checkBoxForInitiated->checkState() == Qt::Checked )
        sq = sq + ",direction_users";
    sq = sq + " where ( (coalesce(:deleted,-1) = -1) or (directions.deleted=:deleted)  )";
    sq = sq + " and directions.ddate between :dfrom and :dto";
    //" and ( (coalesce(:idn_user,-1) = -1) or (direction_users.idn_direction=directions.idn and direction_users.idn_user=:idn_user and direction_users.initiated=0) )";

    if ( ui->lineEditFind->text().trimmed().count() > 0 )
    {
        sq = sq + " and directions.subject like '%'||:subject||'%'";
    }

    bool isok = false;
    ui->lineEditNumDoc->text().trimmed().toInt(&isok);
    if ( ui->lineEditNumDoc->text().trimmed().count() > 0 &&
         isok)
    {
        sq = sq + " and directions.num like '%'||:num||'%'";
    }

    if ( ui->checkBoxForInitiated->checkState() == Qt::Checked )
        sq = sq + " and direction_users.idn_direction=directions.idn and direction_users.idn_user=:idn_user and direction_users.initiated=0";

    sq = sq + ";";
    query.prepare(sq);

    if ( ui->lineEditFind->text().trimmed().count() > 0 )
    {
        query.bindValue(":subject",ui->lineEditFind->text().trimmed().remove(1024,ui->lineEditFind->text().length()));
    }

    if ( ui->lineEditNumDoc->text().trimmed().count() > 0 &&
         isok)
    {
        query.bindValue(":num",ui->lineEditNumDoc->text().trimmed().toInt());
    }

    query.bindValue(":dfrom",ui->dateEditFrom->date());
    query.bindValue(":dto",ui->dateEditTo->date());


    if ( ui->checkBoxShowDeleted->checkState() == Qt::Checked )
        query.boundValue(":deleted").clear();
    else
        query.bindValue(":deleted",QVariant(0));

    if ( ui->checkBoxForInitiated->checkState() == Qt::Checked )
        query.bindValue(":idn_user",currentUserIdn);


    if ( !query.exec() )
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());

    //adding records to table
    while (query.next())
    {
        n = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(n);
        QTableWidgetItem *q = new QTableWidgetItem(query.value("num").toString());
        q->setData(DataRole::idn,query.value("idn").toInt());
        q->setData(DataRole::deleted,query.value("deleted").toInt());
        ui->tableWidget->setItem(n, 0, q);
        ui->tableWidget->setItem(n, 1, new QTableWidgetItem(query.value("fdate").toString()));
        ui->tableWidget->setItem(n, 2, new QTableWidgetItem(query.value("subject").toString()));

        if ( query.value("deleted").toInt() )
            for( int i = 0; i < ui->tableWidget->columnCount(); i++)
                ui->tableWidget->item(n,i)->setForeground(Qt::darkGray);
    }
    db.close();

    ui->tableWidget->setSortingEnabled(true);
    //restore selected item if need
    if ( selectedIdn != -1 )
    {
        n = ui->tableWidget->rowCount();
        for( int i = 0; i < n; i++ )
            if ( QVariant(ui->tableWidget->item(i,0)->data(DataRole::idn)).toInt() == selectedIdn )
            {
                ui->tableWidget->selectRow(i);
                break;
            }
    }
}

void MainWindow::configTable()
{
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("№"));
    ui->tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Дата"));
    ui->tableWidget->setHorizontalHeaderItem(2,new QTableWidgetItem("Тема"));

    //ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);//Fixed
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);//Fixed
}

void MainWindow::on_pushButtonFind_clicked()
{
    refreshTable();
}

void MainWindow::on_pushButtonEdit_clicked()
{
    editDirection();
}

void MainWindow::addNewDirection()
{
    DialogDirection* dialogDirectionWindow = new DialogDirection;
    dialogDirectionWindow->setModal(true);
    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogDirectionWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(&this->db);
    connect(this, SIGNAL(sendDirectionIdn(int)), dialogDirectionWindow, SLOT(recieveDirectionIdn(int)));
    emit sendDirectionIdn(-1);
    connect(this,SIGNAL(sendUserPermissions(int)),dialogDirectionWindow, SLOT(recieveUserPermissions(int)));
    emit sendUserPermissions(this->currentUserRights);
    connect(this,SIGNAL(sendCurrentUserIdn(int)),dialogDirectionWindow,SLOT(recieveCurrentUserIdn(int)));
    emit sendCurrentUserIdn(currentUserIdn);
    connect(this,SIGNAL(sendSettingsApp(QSettings**)),dialogDirectionWindow,SLOT(recieveSettingsApp(QSettings**)));
    emit sendSettingsApp(&settingsApp);
    dialogDirectionWindow->exec();
    dialogDirectionWindow->~DialogDirection();
    dialogDirectionWindow = NULL;
    refreshTable();
}

void MainWindow::editDirection()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        DialogDirection* dialogDirectionWindow = new DialogDirection;
        dialogDirectionWindow->setModal(true);
        connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogDirectionWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
        emit sendDbSettings(&this->db);
        connect(this, SIGNAL(sendDirectionIdn(int)), dialogDirectionWindow, SLOT(recieveDirectionIdn(int)));
        emit sendDirectionIdn(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(DataRole::idn).toInt());
        connect(this,SIGNAL(sendUserPermissions(int)),dialogDirectionWindow, SLOT(recieveUserPermissions(int)));
        emit sendUserPermissions(this->currentUserRights);
        connect(this,SIGNAL(sendCurrentUserIdn(int)),dialogDirectionWindow,SLOT(recieveCurrentUserIdn(int)));
        emit sendCurrentUserIdn(currentUserIdn);
        connect(this,SIGNAL(sendSettingsApp(QSettings**)),dialogDirectionWindow,SLOT(recieveSettingsApp(QSettings**)));
        emit sendSettingsApp(&settingsApp);
        dialogDirectionWindow->exec();
        dialogDirectionWindow->~DialogDirection();
        dialogDirectionWindow = NULL;
        refreshTable();
    } else
            QMessageBox::information(this, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}

void MainWindow::deleteDirection()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        if (!db.open()) { QMessageBox::critical(this, tr("Database Error"), "last error:" + db.lastError().text()); }
        QSqlQuery query(db);

        query.prepare("UPDATE directions SET deleted=1 where idn=:idn");
        query.bindValue(":idn",QVariant(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(DataRole::idn)).toInt());
        if ( !query.exec() )
            QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db.close();
        refreshTable();
    } else
            QMessageBox::information(this, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}

void MainWindow::on_pushButtonDelete_clicked()
{
    deleteDirection();
}

void MainWindow::createContextTableMenu()
{
    contextTableMenu->clear();
    contextTableMenu->addAction(QIcon(":/images/icon/add.png"),"Добавить");
    contextTableMenu->addAction(QIcon(":/images/icon/edit.png"),"Править/Просмотр");
    contextTableMenu->addAction(QIcon(":/images/icon/remove.png"),"Удалить");
    contextTableMenu->addSeparator();
    contextTableMenu->addAction(QIcon(":/images/icon/restore.png"),"Восстановить");
}

void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *q = ui->tableWidget->itemAt(pos);
    if ( q == NULL ) return;

    ui->tableWidget->selectRow(q->row()); // fix bug with quick right click

    contextTableMenu->actions().at(2)->setEnabled( ( QVariant(ui->tableWidget->item(q->row(),0)->data(DataRole::deleted)).toInt() == 0 )
                                                   and Act::userPermission(Act::editDirection,currentUserRights));
    contextTableMenu->actions().at(4)->setEnabled( ( QVariant(ui->tableWidget->item(q->row(),0)->data(DataRole::deleted)).toInt() == 1 )
                                                   and Act::userPermission(Act::editDirection,currentUserRights));

    QAction* selectedItem = contextTableMenu->exec(QCursor::pos());
    if ( !selectedItem ) return;
    if ( selectedItem->text() == "Добавить")
    {
        addNewDirection();
    } else if ( selectedItem->text() == "Править/Просмотр")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        editDirection();
    } else if ( selectedItem->text() == "Удалить")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        deleteDirection();
    } else if ( selectedItem->text() == "Восстановить")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        restoreDirection();
    }
}

void MainWindow::restoreDirection()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        if (!db.open()) { QMessageBox::critical(this, tr("Database Error"), "last error:" + db.lastError().text()); }
        QSqlQuery query(db);

        query.prepare("UPDATE directions SET deleted=0 where idn=:idn");
        query.bindValue(":idn",QVariant(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(DataRole::idn)).toInt());
        if ( !query.exec() )
            QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db.close();
        refreshTable();
    } else
            QMessageBox::information(this, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}


void MainWindow::on_tableWidget_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    editDirection();
}

void MainWindow::recieveAuthorizedUserIdn(int userIdn)
{
    this->currentUserIdn = userIdn;
    if ( userIdn == ApplicationConfiguration::idnGuest )
    {
        this->currentUserRights = UserRights::readonly;
        statusBar()->addPermanentWidget(new QLabel("Пользователь: Гость",this));
        ui->actionChangePassword->setEnabled(false);
    }
    else
    {
        if (!db.open()) { QMessageBox::critical(this, tr("Database Error"), "last error:" + db.lastError().text()); }
        QSqlQuery query(db);
        query.prepare("select users.idn, users.surname || ' ' || LEFT(users.name,1) || '. ' || LEFT(users.patronymic,1) || '.' as initials,spr_job.job,users.deleted,users.permissions,users.tab_number from users,spr_job"
                      " where spr_job.idn = users.idn_job and users.idn=:idn;");
        query.bindValue(":idn",currentUserIdn);
        if ( !query.exec() )
            QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());

        if (query.next())
        {
            if (query.value("permissions").toInt() == UserRights::user)
                this->currentUserRights = UserRights::user;
            else if (query.value("permissions").toInt() == UserRights::writer)
                this->currentUserRights = UserRights::writer;
            else if (query.value("permissions").toInt() == UserRights::admin)
                this->currentUserRights = UserRights::admin;
            else
                this->currentUserRights = UserRights::readonly;
            statusBar()->addPermanentWidget(new QLabel("Пользователь: " + query.value("initials").toString() +
                                                   " " + query.value("job").toString()
                                            ,this));
            currentUserLogin = query.value("tab_number").toString();
        }
        db.close();
    }
    enableControls(Act::userPermission(Act::editDirection,this->currentUserRights));
    refreshTable();
}

void MainWindow::enableControls(bool enable)
{
    ui->pushButtonAdd->setEnabled(enable);
    ui->pushButtonDelete->setEnabled(enable);

    contextTableMenu->actions().at(0)->setEnabled(enable);
    contextTableMenu->actions().at(2)->setEnabled(enable);
    contextTableMenu->actions().at(4)->setEnabled(enable);
}



void MainWindow::on_comboBoxDate_currentIndexChanged(int index)
{
    ui->dateEditFrom->setEnabled(index == 0);
    ui->dateEditTo->setEnabled(index == 0);

    if ( index == 0 ) return;

    QDate Now;

    if (!db.open()) { QMessageBox::critical(this, tr("Database Error"), "last error:" + db.lastError().text()); }

    QSqlQuery query(db);
    query.prepare("SELECT Cast('NOW' as Date) as ddate FROM RDB$DATABASE;");
    if ( !query.exec() )
        QMessageBox::critical(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    query.next();

    Now = query.value("ddate").toDate();
    db.close();

    if ( index == 1 )
    {
        ui->dateEditFrom->setDate(QDate(Now.year(),Now.month(),Now.day()-Now.dayOfWeek()+1));
        ui->dateEditTo->setDate(Now);
    }
    else if ( index == 2 )
    {
        ui->dateEditFrom->setDate(QDate(Now.year(),Now.month(),1));
        ui->dateEditTo->setDate(Now);
    }
    else if ( index == 3 )
    {
        ui->dateEditFrom->setDate(QDate(Now.year(),1,1));
        ui->dateEditTo->setDate(Now);
    }
    else
    {
        ui->dateEditFrom->setDate(Now);
        ui->dateEditTo->setDate(Now);
    }

}

void MainWindow::showSettingsApp()
{
    DialogOptions *dialogOptionsWindow = new DialogOptions;
    dialogOptionsWindow->setModal(true);

    connect(this,SIGNAL(sendSettingsApp(QSettings**)),dialogOptionsWindow,SLOT(recieveSettingsApp(QSettings**)));
    emit sendSettingsApp(&settingsApp);

    dialogOptionsWindow->exec();

    dialogOptionsWindow->~DialogOptions();
    dialogOptionsWindow = NULL;
}

void MainWindow::saveSettings()
{
    bool saveLastUser;
    bool saveFilter;
    bool savePosition;


    settingsApp->beginGroup("main");
    saveLastUser = settingsApp->value("savelastuser",true).toBool();
    saveFilter = settingsApp->value("savefilter",false).toBool();
    savePosition = settingsApp->value("saveposition",false).toBool();
    settingsApp->endGroup();

    if ( saveLastUser && currentUserIdn != ApplicationConfiguration::idnGuest)
    {
        settingsApp->beginGroup("main");
        settingsApp->setValue("lastuser",currentUserLogin);
        settingsApp->endGroup();
    }

    if ( saveFilter )
    {
        settingsApp->beginGroup("filter");
        settingsApp->setValue("subject",ui->lineEditFind->text());
        settingsApp->setValue("numdoc",ui->lineEditNumDoc->text());
        settingsApp->setValue("datefrom",ui->dateEditFrom->date());
        settingsApp->setValue("dateto",ui->dateEditTo->date());
        settingsApp->setValue("dateswitch",ui->comboBoxDate->currentIndex());
        settingsApp->setValue("onlyforme",ui->checkBoxForInitiated->checkState() == Qt::Checked);
        settingsApp->setValue("showdeleted",ui->checkBoxShowDeleted->checkState() == Qt::Checked);
        settingsApp->setValue("showfilter",ui->widgetExtendedOptions->isVisible());
        settingsApp->endGroup();
    }

    if ( savePosition )
    {
        settingsApp->beginGroup("mainwindow");
        settingsApp->setValue("size", size());
        settingsApp->setValue("pos", pos());
        settingsApp->endGroup();
    }

}

void MainWindow::readMainWindowPositionAndSize()
{
    bool savePosition;
    settingsApp->beginGroup("main");
    savePosition = settingsApp->value("saveposition",false).toBool();
    settingsApp->endGroup();

    if(savePosition)
    {
        settingsApp->beginGroup("mainwindow");
        resize(settingsApp->value("size",QSize(this->width(),this->height())).toSize());
        move(settingsApp->value("pos",QPoint(800,600)).toPoint());
        settingsApp->endGroup();
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    saveSettings();
}

void MainWindow::readMainWindowSettings()
{
    readMainWindowPositionAndSize();
    readMainWindowFilterSettings();
}

void MainWindow::readMainWindowFilterSettings()
{
    bool saveFilter;
    settingsApp->beginGroup("main");
    saveFilter = settingsApp->value("savefilter",false).toBool();
    settingsApp->endGroup();

    if ( saveFilter )
    {

        settingsApp->beginGroup("filter");
        ui->lineEditFind->setText(settingsApp->value("subject","").toString());
        ui->lineEditNumDoc->setText(settingsApp->value("numdoc","").toString());
        ui->comboBoxDate->setCurrentIndex(settingsApp->value("dateswitch",0).toInt());
        if ( ui->comboBoxDate->currentIndex() == 0 )
        {
            ui->dateEditFrom->setDate(settingsApp->value("datefrom",QDate()).toDate());
            ui->dateEditTo->setDate(settingsApp->value("dateto",QDate()).toDate());
        }
        //ui->checkBoxForInitiated->setCheckable();

        ui->checkBoxForInitiated->setChecked(settingsApp->value("onlyforme",false).toBool());
        ui->checkBoxShowDeleted->setChecked(settingsApp->value("showdeleted",false).toBool());

        if ( settingsApp->value("showfilter",false).toBool())
            ui->pushButtonMoreOptions->click();

        settingsApp->endGroup();
    }
}
