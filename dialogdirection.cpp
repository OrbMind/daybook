#include "dialogdirection.h"
#include "ui_dialogdirection.h"

DialogDirection::DialogDirection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDirection)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Создать новое распоряжение");
    contextTableMenu = NULL;
    ui->tabWidget->setCurrentIndex(0);
    ui->pushButtonSetInitiated->setEnabled(false);
    directionIdn = -1;
    idnRequested = -1;
    idnRecorded = -1;

    contextTableMenu = new QMenu(this);
    createContextTableMenu();
}

DialogDirection::~DialogDirection()
{
    if (contextTableMenu != NULL)
    {
        contextTableMenu->~QMenu();
        contextTableMenu = NULL;
    }
    delete ui;
}

void DialogDirection::on_buttonBox_rejected()
{
    this->reject();
}

void DialogDirection::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
}

void DialogDirection::recieveDirectionIdn(int directionIdn)
{
    if ( directionIdn < 0 )
    {
        fillUsersList();
        fillTodayDate();
        fillNewNumber();
        ui->lineEditSubject->setFocus();
    }else
    {
        this->directionIdn = directionIdn;
        if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); }
        QSqlQuery query(*db);
        query.prepare("select num,ddate,subject,text,file,idn_request,idn_recorded from directions where idn=:idn;");
        query.bindValue(":idn",directionIdn);
        if ( !query.exec() )
            QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
        query.next();
        idnRequested = query.value("idn_request").toInt();
        idnRecorded = query.value("idn_recorded").toInt();

        fillUsersList();

        ui->lineEditNum->setText(query.value("num").toString());
        ui->lineEditSubject->setText(query.value("subject").toString());
        ui->plainTextEditText->setPlainText(query.value("text").toString());
        ui->dateEditDdate->setDate(query.value("ddate").toDate());
        ui->lineEditFilePath->setText(query.value("file").toString());

        ui->comboBoxRecorded->setCurrentIndex(
            ui->comboBoxRecorded->findData(QVariant(query.value("idn_recorded").toInt()),Qt::UserRole));
        ui->comboBoxRequest->setCurrentIndex(
            ui->comboBoxRequest->findData(QVariant(query.value("idn_request").toInt()),Qt::UserRole));

        query.clear();
        db->close();

        fillInitiatedUsers();
        this->setWindowTitle(QString(ApplicationConfiguration::briefNameApplication) + ": Редактирование/просмотр распоряжения");
    }
}

void DialogDirection::fillInitiatedUsers()
{
    QListWidgetItem *q;
    if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); }
    QSqlQuery query(*db);
    query.prepare("select a.idn_direction,a.idn_user,a.initiated,"
                  " b.surname || ' ' || LEFT(b.name,1) || '. ' || LEFT(b.patronymic,1) || '.' as initials"
                  " from direction_users a,users b"
                  " where b.idn=a.idn_user and a.idn_direction=:idn_direction;");
    query.bindValue(":idn_direction",directionIdn);
    if ( !query.exec() )
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    while(query.next())
    {
        q = new QListWidgetItem(query.value("initials").toString());
        q->setData(Qt::UserRole,QVariant(query.value("idn_user").toInt()));
        q->setData(Qt::UserRole+1,QVariant(query.value("initiated").toInt()));
        ui->listWidgetToInitiated->addItem(q);
    }
    db->close();

    copyToListWidgetDirectionUsers();
}

void DialogDirection::fillTodayDate()
{
    if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); }
    QSqlQuery query(*db);
    query.prepare("SELECT Cast('NOW' as Date) as ddate FROM RDB$DATABASE;");
    if ( !query.exec() )
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    query.next();
    ui->dateEditDdate->setDate(query.value("ddate").toDate());
    db->close();
}

void DialogDirection::fillNewNumber()
{
    if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); }
    QSqlQuery query(*db);
    query.prepare("select max(num) as num from directions where Extract(year FROM cast(ddate as date))=:year and deleted=0;");
    query.bindValue(":year",ui->dateEditDdate->date().year());
    if ( !query.exec() )
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    query.next();
    //QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    ui->lineEditNum->setText(QString::number(query.value("num").toInt()+1));
    db->close();
}

void DialogDirection::fillUsersList()
{
    QListWidgetItem* q;
    ui->comboBoxRecorded->clear();
    ui->comboBoxRequest->clear();
    ui->listWidgetAllUsers->clear();


    if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); }
    QSqlQuery query(*db);

    query.prepare("select users.idn, users.surname || ' ' || LEFT(users.name,1) || '. ' || LEFT(users.patronymic,1) || '.' as initials,spr_job.job,users.deleted from users,spr_job"
                  " where spr_job.idn = users.idn_job and ( users.deleted=0 or users.idn = :idn1 or users.idn = :idn2)"
                  " ORDER BY users.surname ASC;");
    query.bindValue(":idn1",idnRecorded);
    query.bindValue(":idn2",idnRequested);
    if ( !query.exec() )
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    while( query.next() )
    {
        ui->comboBoxRecorded->addItem(query.value("initials").toString(),QVariant(query.value("idn").toInt()));
        ui->comboBoxRequest->addItem(query.value("initials").toString(),QVariant(query.value("idn").toInt()));

        q = new QListWidgetItem(query.value("initials").toString());
        q->setData(Qt::UserRole,QVariant(query.value("idn").toInt()));
        ui->listWidgetAllUsers->addItem(q);
    }
    db->close();

    //colourListWidget();
}

void DialogDirection::on_pushButtonSetSelectedUsers_clicked()
{
    int i = 0;
    foreach( QListWidgetItem *item, ui->listWidgetAllUsers->selectedItems())
    {
        for( i = 0; i < ui->listWidgetToInitiated->count(); ++i )
            if ( QVariant(ui->listWidgetToInitiated->item(i)->data(Qt::UserRole)).toInt() ==
                 QVariant(item->data(Qt::UserRole)).toInt())
                break;
        if( i == ui->listWidgetToInitiated->count())
        ui->listWidgetToInitiated->insertItem(ui->listWidgetToInitiated->count(),
                                              item->clone());
        /*if ( QVariant(ui->listWidgetToInitiated->item(
                          ui->listWidgetToInitiated->count()-1)->data(Qt::UserRole+1)).toInt() == 1 )
            ui->listWidgetToInitiated->item(ui->listWidgetToInitiated->count()-1)->setForeground(Qt::darkRed);
                    //q->setForeground(Qt::darkGray);*/
    }

    copyToListWidgetDirectionUsers();
}

void DialogDirection::on_pushButtonSetAllUsers_clicked()
{
    ui->listWidgetToInitiated->clear();
    for( int i=0; i < ui->listWidgetAllUsers->count(); i++ )
        ui->listWidgetToInitiated->insertItem(ui->listWidgetToInitiated->count(),
                                              ui->listWidgetAllUsers->item(i)->clone());
    copyToListWidgetDirectionUsers();
}

void DialogDirection::copyToListWidgetDirectionUsers()
{
    ui->listWidgetDirectionUsers->clear();
    for( int i = 0; i < ui->listWidgetToInitiated->count(); ++i )
    {
        ui->listWidgetDirectionUsers->insertItem(ui->listWidgetDirectionUsers->count(),
                                                 ui->listWidgetToInitiated->item(i)->clone());
    }
    colourListWidget();
}

void DialogDirection::on_pushButtonRemoveAllUsers_clicked()
{
        ui->listWidgetToInitiated->clear();
        copyToListWidgetDirectionUsers();
}

void DialogDirection::on_pushButtonRemoveSelectedUsers_clicked()
{
    foreach( QListWidgetItem *item, ui->listWidgetToInitiated->selectedItems())
        delete item;
    copyToListWidgetDirectionUsers();
}

void DialogDirection::on_pushButtonChooseFilePath_clicked()
{
    ui->lineEditFilePath->setText(QFileDialog::getOpenFileName(this, tr("Выбрать файл:"),".",tr("Файл *.* (*.*)")));
}

void DialogDirection::on_lineEditFilePath_textChanged(const QString &arg1)
{
    ui->pushButtonFile->setEnabled(arg1.trimmed().length());
}

void DialogDirection::saveInitiatedUsers()
{
    if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); return; }

    QSqlQuery query(*db);
    query.prepare("DELETE from direction_users where idn_direction=:idn_direction;");
    query.bindValue(":idn_direction",directionIdn);
    if ( !query.exec() )
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    query.clear();

    //QMessageBox::critical(0, tr("Query Error"), QString::number(ui->listWidgetToInitiated->count()));
    for( int i = 0; i < ui->listWidgetToInitiated->count(); i++ )
    {
        query.clear();
        query.prepare("INSERT INTO direction_users (idn_direction,idn_user,initiated) VALUES (:idn_direction,:idn_user,:initiated);");
        query.bindValue(":idn_direction",directionIdn);
        query.bindValue(":initiated",QVariant(ui->listWidgetToInitiated->item(i)->data(Qt::UserRole+1)).toInt());
        query.bindValue(":idn_user",QVariant(ui->listWidgetToInitiated->item(i)->data(Qt::UserRole)).toInt());
        if ( !query.exec() )
            QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    }

    db->close();

}

void DialogDirection::on_buttonBox_accepted()
{
    if ( !Act::userPermission(Act::edit,currentUserRights) ) return;
    if ( ! CheckInput() ) return;

    if (!db->open()) { QMessageBox::critical(0, tr("Database Error"), db->lastError().text()); return; }

    QSqlQuery query(*db);

    if (this->directionIdn == -1 )
    {
        query.prepare("select GEN_ID(gen_directions, 1) as idn from RDB$DATABASE;");
                if ( !query.exec() )
                {
                    QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
                    return;
                }
        query.next();
        directionIdn=query.value("idn").toInt();
        query.clear();

        query.prepare("insert into directions (idn,num,ddate,subject,text,idn_request,idn_recorded,file) VALUES (:idn,:num,:ddate,:subject,:text,:idn_request,:idn_recorded,:file);");
    }else
        query.prepare("UPDATE directions SET num=:num,ddate=:ddate,subject=:subject,text=:text,"
                      " idn_request=:idn_request,idn_recorded=:idn_recorded,file=:file WHERE idn=:idn;");

    query.bindValue(":idn",directionIdn);
    query.bindValue(":num",ui->lineEditNum->text().trimmed().toInt());
    query.bindValue(":ddate",ui->dateEditDdate->date());
    query.bindValue(":subject",ui->lineEditSubject->text().trimmed().remove(64,ui->lineEditSubject->text().length()));
    query.bindValue(":text",ui->plainTextEditText->toPlainText().trimmed().remove(256,ui->plainTextEditText->toPlainText().length()));
    query.bindValue(":idn_request",ui->comboBoxRequest->currentData(Qt::UserRole).toInt());
    query.bindValue(":idn_recorded",ui->comboBoxRecorded->currentData(Qt::UserRole).toInt());
    query.bindValue(":file",ui->lineEditFilePath->text().trimmed().remove(1024,ui->lineEditFilePath->text().length()));
    if ( !query.exec() )
        QMessageBox::critical(0, tr("Query Error"), query.lastQuery() + "\n\n" + query.lastError().text());
    db->close();
    saveInitiatedUsers();
    this->accept();

}

bool DialogDirection::CheckInput()
{
    bool isok = false;

    ui->lineEditNum->text().trimmed().toInt(&isok);
    if ( !isok )
    {
        QMessageBox::warning(0, tr("Ошибка"), "В поле номер распоряжения необходимо ввести число.");
        ui->lineEditNum->setFocus();
        return false;
    }

    if ( ui->lineEditSubject->text().trimmed().length() == 0 )
    {
        QMessageBox::warning(0, tr("Ошибка"), "Не заполнено поле тема.");
        ui->lineEditSubject->setFocus();
        return false;
    }

    if ( ui->plainTextEditText->toPlainText().trimmed().length() == 0 )
    {
        QMessageBox::warning(0, tr("Ошибка"), "Не заполнено поле текст.");
        ui->plainTextEditText->setFocus();
        return false;
    }

    if( ui->comboBoxRequest->count() < 1 || ui->comboBoxRecorded->count() < 1)
    {
        QMessageBox::warning(0, tr("Ошибка"), "Не заполнено поле записавшего/разрешившего пользователя");
        ui->comboBoxRequest->setFocus();
        return false;
    }

    return true;
}

void DialogDirection::createContextTableMenu()
{
    contextTableMenu->clear();
    contextTableMenu->addAction(QIcon(":/images/icon/initiated.png"),"Ознакомлен");
    contextTableMenu->addSeparator();
    contextTableMenu->addAction(QIcon(":/images/icon/notinitiated.png"),"Не ознакомлен");
}

void DialogDirection::on_listWidgetDirectionUsers_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* q = ui->listWidgetDirectionUsers->itemAt(pos);

    if ( q == NULL ) return;
    q->setSelected(true); // fix bug with quick click

    contextTableMenu->actions().at(0)->setEnabled( ( QVariant(q->data(Qt::UserRole+1)).toInt() == 0 )
                                                   and Act::userPermission(Act::initiate,currentUserRights));
    contextTableMenu->actions().at(2)->setEnabled( ( QVariant(q->data(Qt::UserRole+1)).toInt() == 1 )
                                                   and Act::userPermission(Act::initiate,currentUserRights));

    QAction* selectedItem = contextTableMenu->exec(QCursor::pos());
    if ( !selectedItem ) return;

    if ( selectedItem->text() == "Ознакомлен" )
    {
        q->setData(Qt::UserRole+1,QVariant(1));
    }else if ( selectedItem->text() == "Не ознакомлен")
    {
        q->setData(Qt::UserRole+1,QVariant(0));
    }

    for ( int i = 0; i < ui->listWidgetToInitiated->count(); ++ i)
        if ( QVariant(q->data(Qt::UserRole)).toInt() ==
             QVariant( ui->listWidgetToInitiated->item(i)->data(Qt::UserRole)).toInt() )
        {
            ui->listWidgetToInitiated->item(i)->setData(Qt::UserRole+1,q->data(Qt::UserRole+1));
            break;
        }
    colourListWidget();
}

void DialogDirection::colourListWidget()
{
    for ( int i = 0; i < ui->listWidgetDirectionUsers->count(); ++i )
        if( QVariant(ui->listWidgetDirectionUsers->item(i)->data(Qt::UserRole+1)).toInt() == 1 )
            ui->listWidgetDirectionUsers->item(i)->setForeground(Qt::darkGreen);
        else if( QVariant(ui->listWidgetDirectionUsers->item(i)->data(Qt::UserRole+1)).toInt() == 0 )
            ui->listWidgetDirectionUsers->item(i)->setForeground(Qt::black);
}

void DialogDirection::on_pushButtonFile_clicked()
{
    QDesktopServices::openUrl(QUrl(ui->lineEditFilePath->text()));
}

void DialogDirection::recieveUserPermissions(int userPermissions)
{
    this->currentUserRights = userPermissions;
    enableControls(Act::userPermission(Act::edit,userPermissions));
}

void DialogDirection::enableControls(bool enable)
{
    ui->lineEditNum->setEnabled(enable);
    ui->dateEditDdate->setEnabled(enable);
    ui->lineEditSubject->setEnabled(enable);
    ui->plainTextEditText->setEnabled(enable);
    ui->comboBoxRequest->setEnabled(enable);
    ui->comboBoxRecorded->setEnabled(enable);

    //ui->listWidgetDirectionUsers->setEnabled(enable);

    ui->pushButtonSetSelectedUsers->setEnabled(enable);
    ui->pushButtonSetAllUsers->setEnabled(enable);
    ui->pushButtonRemoveSelectedUsers->setEnabled(enable);
    ui->pushButtonRemoveAllUsers->setEnabled(enable);
    ui->lineEditFilePath->setReadOnly(!enable);
    ui->pushButtonChooseFilePath->setEnabled(enable);
}

void DialogDirection::enableControlInitiate(bool enable)
{
    //ui->pushButtonSetInitiated->setEnabled(Act::userPermission(Act::initiate,currentUserRights));
}
