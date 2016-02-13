#include "dialogjob.h"
#include "ui_dialogjob.h"

//INSERT INTO spr_job (idn,job) VALUES (GEN_ID(gen_spr_job_idn, 1), 'директор');
//INSERT INTO t1(field1) VALUES('my stuff') RETURNING id;

DialogJob::DialogJob(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogJob)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Справочник должностей");
    contextTableMenu = NULL;
    contextTableMenu = new QMenu(this);
    createContextTableMenu();
    configTable();
    ui->pushButtonFindClear->setVisible(false);
}

DialogJob::~DialogJob()
{
    if (contextTableMenu!=NULL)
    {
        contextTableMenu->~QMenu();
        contextTableMenu = NULL;
    }
    delete ui;
}

void DialogJob::on_pushButtonClose_clicked()
{
   this->close();
}

void DialogJob::configTable()
{
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Должность"));
    ui->tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("№"));
}

void DialogJob::refreshTable()
{
    //remember selected item
    int selectedIdn = -1;
    if ( ui->tableWidget->selectedItems().count() > 0)
        selectedIdn = ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(Qt::UserRole).toInt();
    ui->tableWidget->setSortingEnabled(false);
    //clear table
    int n = ui->tableWidget->rowCount();
    for( int i = 0; i < n; i++ ) ui->tableWidget->removeRow(0);
    //try to open database
    if (!db->open()) { QMessageBox::warning(this, tr("Database Error"), db->lastError().text()); }
    //prepare query for all records, or apply filter
    QSqlQuery query(*db);
    if ( ui->lineEditFind->text().trimmed() != "" )
    {
        query.prepare("select idn,job,deleted,num from spr_job WHERE job like '%'||:sjob||'%' and idn>0;");
        query.bindValue(":sjob", ui->lineEditFind->text().trimmed());
    }
    else
        query.prepare("select idn,job,deleted,num from spr_job WHERE idn>0;");
    //execute query
    if ( !query.exec() )
        QMessageBox::warning(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    //adding records to table
    while (query.next())
    {
        n = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(n);
        QTableWidgetItem *q = new QTableWidgetItem(query.value("job").toString());
        q->setData(DataRole::idn,query.value("idn").toInt());
        q->setData(DataRole::deleted,query.value("deleted").toInt());
        if ( query.value("deleted").toInt() )
            q->setForeground(Qt::darkGray);
        ui->tableWidget->setItem(n, 0, q);
        ui->tableWidget->setItem(n,1, new QTableWidgetItem(query.value("num").toString()));

    }
    db->close();

    ui->tableWidget->setSortingEnabled(true);
    //restore selected item if need
    if ( selectedIdn != -1 )
    {
        n = ui->tableWidget->rowCount();
        for( int i = 0; i < n; i++ )
            if ( ui->tableWidget->item(i,0)->data(DataRole::idn).toInt() == selectedIdn )
            {
                ui->tableWidget->setCurrentCell(i,0);
                break;
            }
    }
}

void DialogJob::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
    refreshTable();
}

void DialogJob::addNewJob()
{
    DialogJobEdit* dialogJobEditWinow = new DialogJobEdit;
    dialogJobEditWinow->setModal(true);
    connect(this,SIGNAL(sendDbSettings(QSqlDatabase*)),dialogJobEditWinow,SLOT(recieveDbSettings(QSqlDatabase*)));
    connect(this,SIGNAL(sendUserPermissions(int)),dialogJobEditWinow, SLOT(recieveUserPermissions(int)));
    emit sendDbSettings(db);
    emit sendUserPermissions(this->currentUserRights);
    if( dialogJobEditWinow->exec() == QDialog::Accepted)
        refreshTable();
    dialogJobEditWinow->~DialogJobEdit();
    dialogJobEditWinow = NULL;
}

void DialogJob::editJob()
{
    DialogJobEdit* dialogJobEditWinow = new DialogJobEdit;
    dialogJobEditWinow->setModal(true);
    connect(this,SIGNAL(sendDbSettings(QSqlDatabase*)),dialogJobEditWinow,SLOT(recieveDbSettings(QSqlDatabase*)));
    connect(this,SIGNAL(sendUserPermissions(int)),dialogJobEditWinow, SLOT(recieveUserPermissions(int)));
    connect(this, SIGNAL(sendJobIdn(int)), dialogJobEditWinow, SLOT(recieveJobIdn(int)));
    emit sendDbSettings(db);
    emit sendUserPermissions(this->currentUserRights);
    emit sendJobIdn(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(DataRole::idn).toInt());
    if( dialogJobEditWinow->exec() == QDialog::Accepted)
        refreshTable();
    dialogJobEditWinow->~DialogJobEdit();
    dialogJobEditWinow = NULL;
}

void DialogJob::deleteJob()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        if (!db->open()) { QMessageBox::warning(this, tr("Database Error"), db->lastError().text()); }
        QSqlQuery query(*db);
        query.prepare("UPDATE spr_job SET deleted=1 WHERE idn=:idn");
        query.bindValue(":idn",QString::number(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(DataRole::idn).toInt()));
        if ( !query.exec() )
            QMessageBox::warning(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        refreshTable();
    } else
        QMessageBox::information(this, tr("Внимание!"), "Необходимо выбрать строку для удаления.");
}

void DialogJob::restoreJob()
{
    if ( ui->tableWidget->selectedItems().count() )
    {
        if (!db->open()) { QMessageBox::warning(this, tr("Database Error"), db->lastError().text()); }
        QSqlQuery query(*db);
        query.prepare("UPDATE spr_job SET deleted=0 WHERE idn=:idn");
        query.bindValue(":idn",QString::number(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->data(DataRole::idn).toInt()));
        if ( !query.exec() )
            QMessageBox::warning(this, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        refreshTable();
    } else
        QMessageBox::information(this, tr("Внимание!"), "Необходимо выбрать строку для восстановления.");
}

void DialogJob::on_pushButtonAddJob_clicked()
{
    addNewJob();
}

void DialogJob::on_pushButtonEditJob_clicked()
{
    if ( ui->tableWidget->selectedItems().count() )
        editJob();
    else
        QMessageBox::information(this, tr("Внимание!"), "Необходимо выбрать строку для редактирования.");
}

void DialogJob::on_pushButtonDeleteJob_clicked()
{
    deleteJob();
}

void DialogJob::on_pushButtonFind_clicked()
{
    refreshTable();
}

void DialogJob::createContextTableMenu()
{
    contextTableMenu->clear();
    contextTableMenu->addAction(QIcon(":/images/icon/add.png"),"Добавить");
    contextTableMenu->addAction(QIcon(":/images/icon/edit.png"),"Править");
    contextTableMenu->addAction(QIcon(":/images/icon/remove.png"),"Удалить");
    contextTableMenu->addSeparator();
    contextTableMenu->addAction(QIcon(":/images/icon/restore.png"),"Восстановить");
}

void DialogJob::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *q = ui->tableWidget->itemAt(pos);
    if ( q == NULL ) return;
    q->setSelected(true); // fix bug with quick click

    contextTableMenu->actions().at(2)->setEnabled( ( QVariant(ui->tableWidget->item(q->row(),0)->data(DataRole::deleted)).toInt() == 0 )
                                                   and Act::userPermission(Act::editDirection,currentUserRights));
    contextTableMenu->actions().at(4)->setEnabled( ( QVariant(ui->tableWidget->item(q->row(),0)->data(DataRole::deleted)).toInt() == 1 )
                                                   and Act::userPermission(Act::editDirection,currentUserRights));

    QAction* selectedItem = contextTableMenu->exec(QCursor::pos());
    if ( !selectedItem ) return;
    if ( selectedItem->text() == "Добавить")
    {
        addNewJob();
    } else if ( selectedItem->text() == "Править")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        editJob();
    } else if ( selectedItem->text() == "Удалить")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        deleteJob();
    } else if ( selectedItem->text() == "Восстановить")
    {
        ui->tableWidget->setCurrentCell(q->row(),q->column());
        restoreJob();
    }

}

void DialogJob::recieveUserPermissions(int userPermissions)
{
    this->currentUserRights = userPermissions;
    enableControls(Act::userPermission(Act::editJob,userPermissions));
}

void DialogJob::enableControls(bool enable)
{
    ui->pushButtonAddJob->setEnabled(enable);
    ui->pushButtonDeleteJob->setEnabled(enable);

    contextTableMenu->actions().at(0)->setEnabled(enable);
    contextTableMenu->actions().at(2)->setEnabled(enable);
    contextTableMenu->actions().at(4)->setEnabled(enable);
}

void DialogJob::on_tableWidget_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    editJob();
}
