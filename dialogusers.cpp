#include "dialogusers.h"
#include "ui_dialogusers.h"

DialogUsers::DialogUsers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogUsers)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Справочник пользователей");
    contextTableMenu = NULL;
    currentUserIdn = -1;
    contextTableMenu = new QMenu(this);
    createContextTableMenu();
    configTable();
}

DialogUsers::~DialogUsers()
{
    if ( contextTableMenu != NULL )
    {
        contextTableMenu->~QMenu();
        contextTableMenu = NULL;
    }
    delete ui;
}

void DialogUsers::configTable()
{
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("ФИО"));
    ui->tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Должность"));
}

void DialogUsers::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
    refreshTable();
}

void DialogUsers::on_pushButtonClose_clicked()
{
    this->close();
}

void DialogUsers::refreshTable()
{
    //remember selected item
    int selectedIdn = -1;
    if ( ui->tableWidget->selectedItems().count() > 0)
        selectedIdn = ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole).toInt();
    //clear table
    int n = ui->tableWidget->rowCount();
    for( int i =0; i < n; i++ ) ui->tableWidget->removeRow(0);
    //try to open database
    if (!db->open()) { QMessageBox::warning(0, tr("Database Error"), db->lastError().text()); }
    //prepare query for all records, or apply filter
    QSqlQuery query(*db);
    if ( ui->lineEditFind->text().trimmed() != "" )
    {
        query.prepare("select users.idn, users.surname || ' ' || LEFT(users.name,1) || '. ' || LEFT(users.patronymic,1) || '.' as initials,spr_job.job,users.deleted from users,spr_job"
                      " where spr_job.idn = users.idn_job and ( users.surname like '%'||:sfind||'%' or spr_job.job like '%'||:sfind||'%');");
        query.bindValue(":sfind", ui->lineEditFind->text().trimmed());
    }
    else
        query.prepare("select users.idn, users.surname || ' ' || LEFT(users.name,1) || '. ' || LEFT(users.patronymic,1) || '.' as initials,spr_job.job,users.deleted from users,spr_job where spr_job.idn = users.idn_job;");
    //execute query
    if ( !query.exec() )
        QMessageBox::warning(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    //adding records to table
    while (query.next())
    {
        n = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(n);
        QTableWidgetItem *q = new QTableWidgetItem(query.value(1).toString());
        q->setData(Qt::UserRole,query.value(0).toInt());
        //if ( query.value("deleted").toInt() )
        //    q->setForeground(Qt::darkGray);
        ui->tableWidget->setItem(n, 0, q);
        ui->tableWidget->setItem(n,1,new QTableWidgetItem(query.value(2).toString()));
        if ( query.value("deleted").toInt() )
            for( int i = 0; i < ui->tableWidget->columnCount(); i++)
                ui->tableWidget->item(n,i)->setForeground(Qt::darkGray);
    }
    db->close();
    //restore selected item if need
    if ( selectedIdn != -1 )
    {
        n = ui->tableWidget->rowCount();
        for( int i = 0; i < n; i++ )
            if ( ui->tableWidget->item(i,0)->data(Qt::UserRole).toInt() == selectedIdn )
            {
                ui->tableWidget->setCurrentCell(i,0);
                break;
            }
    }
}

void DialogUsers::addNewUser()
{
    DialogUserEdit* dialogUserEditWindow = new DialogUserEdit;
    dialogUserEditWindow->setModal(true);
    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogUserEditWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(this->db);
    connect(this,SIGNAL(sendUserPermissions(int)),dialogUserEditWindow, SLOT(recieveUserPermissions(int)));
    emit sendUserPermissions(this->currentUserRights);

    connect(this,SIGNAL(sendUserPermissions(int)),dialogUserEditWindow, SLOT(recieveUserPermissions(int)));
    emit sendUserPermissions(this->currentUserRights);

    connect(this,SIGNAL(sendCurrentUserIdn(int)),dialogUserEditWindow,SLOT(recieveCurrentUserIdn(int)));
    emit sendCurrentUserIdn(currentUserIdn);

    //connect(dialogJobEditWinow, SIGNAL(sendEditJob(QString,int,bool)), this, SLOT(recieveEditJob(QString,int,bool)) );
    dialogUserEditWindow->exec();
    dialogUserEditWindow->~DialogUserEdit();
    dialogUserEditWindow = NULL;
    refreshTable();
}

void DialogUsers::editUser()
{
    DialogUserEdit* dialogUserEditWindow = new DialogUserEdit;
    dialogUserEditWindow->setModal(true);
    connect(this, SIGNAL(sendDbSettings(QSqlDatabase*)), dialogUserEditWindow, SLOT(recieveDbSettings(QSqlDatabase*)));
    emit sendDbSettings(this->db);
    connect(this, SIGNAL(sendUserIdn(int)), dialogUserEditWindow, SLOT(recieveUserIdn(int)) );
    emit sendUserIdn(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole).toInt());
    connect(this,SIGNAL(sendUserPermissions(int)),dialogUserEditWindow, SLOT(recieveUserPermissions(int)));
    emit sendUserPermissions(this->currentUserRights);
    connect(this,SIGNAL(sendCurrentUserIdn(int)),dialogUserEditWindow,SLOT(recieveCurrentUserIdn(int)));
    emit sendCurrentUserIdn(currentUserIdn);

    dialogUserEditWindow->exec();
    dialogUserEditWindow->~DialogUserEdit();
    dialogUserEditWindow = NULL;
    refreshTable();
}


void DialogUsers::deleteUser()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); }
        QSqlQuery query(*db);
        query.prepare("UPDATE users SET deleted=1 WHERE idn=:idn");
        query.bindValue(":idn",QString::number(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole).toInt()));
        if ( !query.exec() )
            QMessageBox::warning(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        refreshTable();
    } else
        QMessageBox::information(0, tr("Внимание!"), "Необходимо выбрать строку для удаления.");
    refreshTable();
}

void DialogUsers::restoreUser()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); }
        QSqlQuery query(*db);
        query.prepare("UPDATE users SET deleted=0 WHERE idn=:idn");
        query.bindValue(":idn",QString::number(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole).toInt()));
        if ( !query.exec() )
            QMessageBox::warning(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        refreshTable();
    } else
        QMessageBox::information(0, tr("Внимание!"), "Необходимо выбрать строку для удаления.");
    refreshTable();
}

void DialogUsers::on_pushButtonAdd_clicked()
{
    addNewUser();
}

void DialogUsers::on_pushButtonEdit_clicked()
{
    if ( ui->tableWidget->selectedItems().count() )
        editUser();
    else
        QMessageBox::information(0, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}

void DialogUsers::on_pushButtonDelete_clicked()
{
    deleteUser();
}

void DialogUsers::createContextTableMenu()
{
    contextTableMenu->clear();
    contextTableMenu->addAction(QIcon(":/images/icon/add.png"),"Добавить");
    contextTableMenu->addAction(QIcon(":/images/icon/edit.png"),"Править");
    contextTableMenu->addAction(QIcon(":/images/icon/remove.png"),"Удалить");
    contextTableMenu->addSeparator();
    contextTableMenu->addAction(QIcon(":/images/icon/restore.png"),"Восстановить");
}

void DialogUsers::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *q = ui->tableWidget->itemAt(pos);
    if ( q == NULL ) return;

    QAction* selectedItem = contextTableMenu->exec(QCursor::pos());
    if ( !selectedItem ) return;
    if ( selectedItem->text() == "Добавить")
    {
        addNewUser();
    } else if ( selectedItem->text() == "Править")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        editUser();
    } else if ( selectedItem->text() == "Удалить")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        deleteUser();
    } else if ( selectedItem->text() == "Восстановить")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        restoreUser();
    }
}

void DialogUsers::on_pushButtonFind_clicked()
{
    refreshTable();
}

void DialogUsers::recieveUserPermissions(int userPermissions)
{
    this->currentUserRights = userPermissions;
    enableControls(Act::userPermission(Act::editUser,userPermissions));
}

void DialogUsers::enableControls(bool enable)
{
    ui->pushButtonAdd->setEnabled(enable);
    ui->pushButtonDelete->setEnabled(enable);

    contextTableMenu->actions().at(0)->setEnabled(enable);
    contextTableMenu->actions().at(2)->setEnabled(enable);
    contextTableMenu->actions().at(4)->setEnabled(enable);
}

void DialogUsers::on_tableWidget_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    editUser();
}

void DialogUsers::recieveCurrentUserIdn(int userIdn)
{
    this->currentUserIdn = userIdn;
}
