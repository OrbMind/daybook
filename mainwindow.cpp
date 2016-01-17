#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWindowSettings = NULL;
    contextTableMenu = NULL;
    currentUserRights = UserRights::readonly;

    this->setWindowTitle(QString(ApplicationConfiguration::fullNameApplication));


    createMenuActions();

    ui->widgetExtendedOptions->setVisible(false);
    ui->pushButtonMoreOptions->setIcon(QIcon(":/images/icon/more.png"));

    if( ! configDB() ) exit(1);

    configTable();

    contextTableMenu = new QMenu(this);
    createContextTableMenu();

}

bool MainWindow::enterSoft()
{
    DialogEnterSoft* dialogEnterSoftWindow = new DialogEnterSoft(this);
    dialogEnterSoftWindow->setModal(true);
    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogEnterSoftWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(&this->db);
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
        QMessageBox::critical(0, tr("Main config error"), "Файл конфигурации приложения не найден:\n"+mainConfigFile.fileName()+
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
    //db.setHostName(mainWindowSettings->value("hostname","localhost").toString());
    db.setHostName(mainWindowSettings->value("hostname","10.0.2.200").toString());
    //db.setDatabaseName(mainWindowSettings->value("databasename","daybook").toString());
    db.setDatabaseName(mainWindowSettings->value("databasename","test").toString());
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
    delete ui;
}

void MainWindow::createMenuActions()
{
    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(quitFromApp()));
    connect(ui->actionJob,SIGNAL(triggered()),this,SLOT(showJobSpr()));
    connect(ui->actionUsers,SIGNAL(triggered()),this,SLOT(showUsersSpr()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(showDialogAbout()));
}

void MainWindow::showDialogAbout()
{
    DialogAbout* dialogAboutWindow = new DialogAbout;
    dialogAboutWindow->setModal(true);
    dialogAboutWindow->exec();

    dialogAboutWindow->~DialogAbout();
    dialogAboutWindow = NULL;
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
        selectedIdn = QVariant(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole)).toInt();
    //clear table
    int n = ui->tableWidget->rowCount();
    for( int i =0; i < n; i++ ) ui->tableWidget->removeRow(0);

    if (!db.open()) { QMessageBox::critical(0, tr("Database Error"), "last error:" + db.lastError().text()); }
    QSqlQuery query(db);

    //query.prepare("select idn,num,ddate,subject,deleted from directions;");
    query.prepare("select idn,num,"
    " CAST(lpad(EXTRACT(DAY FROM ddate),2,'0') AS varchar(2))||'.'|| CAST(lpad(EXTRACT(MONTH FROM ddate),2,'0') AS varchar(2))||'.'|| EXTRACT(YEAR FROM ddate) AS fdate"
    " ,subject,deleted from directions where ( (coalesce(:deleted,-1) = -1) or (deleted=:deleted)  );");

    if ( ui->checkBoxShowDeleted->checkState() == Qt::Checked)
        query.boundValue(":deleted").clear();
    else
        query.bindValue(":deleted",QVariant(0));

    if ( !query.exec() )
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());

    //adding records to table
    while (query.next())
    {
        n = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(n);
        QTableWidgetItem *q = new QTableWidgetItem(query.value("num").toString());
        q->setData(Qt::UserRole,query.value("idn").toInt());
        if ( query.value("deleted").toInt() )
            q->setForeground(Qt::darkGray);
        ui->tableWidget->setItem(n, 0, q);
        ui->tableWidget->setItem(n, 1, new QTableWidgetItem(query.value("fdate").toString()));
        ui->tableWidget->setItem(n, 2, new QTableWidgetItem(query.value("subject").toString()));

    }
    db.close();

    //restore selected item if need
    if ( selectedIdn != -1 )
    {
        n = ui->tableWidget->rowCount();
        for( int i = 0; i < n; i++ )
            if ( QVariant(ui->tableWidget->item(i,0)->data(Qt::UserRole)).toInt() == selectedIdn )
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
        emit sendDirectionIdn(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole).toInt());
        connect(this,SIGNAL(sendUserPermissions(int)),dialogDirectionWindow, SLOT(recieveUserPermissions(int)));
        emit sendUserPermissions(this->currentUserRights);
        dialogDirectionWindow->exec();
        dialogDirectionWindow->~DialogDirection();
        dialogDirectionWindow = NULL;
        refreshTable();
    } else
            QMessageBox::information(0, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}

void MainWindow::deleteDirection()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        if (!db.open()) { QMessageBox::critical(0, tr("Database Error"), "last error:" + db.lastError().text()); }
        QSqlQuery query(db);

        query.prepare("UPDATE directions SET deleted=1 where idn=:idn");
        query.bindValue(":idn",QVariant(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole)).toInt());
        if ( !query.exec() )
            QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db.close();
        refreshTable();
    } else
            QMessageBox::information(0, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}

void MainWindow::on_pushButtonDelete_clicked()
{
    deleteDirection();
}

void MainWindow::createContextTableMenu()
{
    contextTableMenu->clear();
    contextTableMenu->addAction(QIcon(":/images/icon/add.png"),"Добавить");
    contextTableMenu->addAction(QIcon(":/images/icon/edit.png"),"Править/Просмотреть");
    contextTableMenu->addAction(QIcon(":/images/icon/remove.png"),"Удалить");
    contextTableMenu->addSeparator();
    contextTableMenu->addAction(QIcon(":/images/icon/restore.png"),"Восстановить");
}

void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *q = ui->tableWidget->itemAt(pos);
    if ( q == NULL ) return;

    ui->tableWidget->selectRow(q->row()); // fix bug with quick right click

    QAction* selectedItem = contextTableMenu->exec(QCursor::pos());
    if ( !selectedItem ) return;
    if ( selectedItem->text() == "Добавить")
    {
        addNewDirection();
    } else if ( selectedItem->text() == "Править/Просмотреть")
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
        if (!db.open()) { QMessageBox::critical(0, tr("Database Error"), "last error:" + db.lastError().text()); }
        QSqlQuery query(db);

        query.prepare("UPDATE directions SET deleted=0 where idn=:idn");
        query.bindValue(":idn",QVariant(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole)).toInt());
        if ( !query.exec() )
            QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        db.close();
        refreshTable();
    } else
            QMessageBox::information(0, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}


void MainWindow::on_tableWidget_doubleClicked(const QModelIndex &index)
{
    editDirection();
}

void MainWindow::recieveAuthorizedUserIdn(int userIdn)
{
    this->currentUserIdn = userIdn;
    if ( userIdn == ApplicationConfiguration::idnGuest )
    {
        this->currentUserRights = UserRights::readonly;
        statusBar()->addPermanentWidget(new QLabel("Пользователь: Гость",this));
    }
    else
    {
        if (!db.open()) { QMessageBox::critical(0, tr("Database Error"), "last error:" + db.lastError().text()); }
        QSqlQuery query(db);
        query.prepare("select users.idn, users.surname || ' ' || LEFT(users.name,1) || '. ' || LEFT(users.patronymic,1) || '.' as initials,spr_job.job,users.deleted,users.permissions from users,spr_job"
                      " where spr_job.idn = users.idn_job and users.idn=:idn;");
        query.bindValue(":idn",currentUserIdn);
        if ( !query.exec() )
            QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());

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
        }
        db.close();
    }
    enableControls(Act::userPermission(Act::edit,this->currentUserRights));
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


