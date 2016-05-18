#include "dialogupdatedb.h"
#include "ui_dialogupdatedb.h"

DialogUpdateDb::DialogUpdateDb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogUpdateDb)
{
    ui->setupUi(this);
}

DialogUpdateDb::~DialogUpdateDb()
{
    delete ui;
}

void DialogUpdateDb::on_pushButtonClose_clicked()
{
    this->close();
}

void DialogUpdateDb::recieveDbSettings(QSqlDatabase *db)
{
    this->db = db;
}

void DialogUpdateDb::on_pushButtonUpdateDB_clicked()
{
    bool result = false;

    result = checkConnection();

    if( result ) result = checkVersion();

    if( result ) result = updateV2V3();

    if( result ) addLog("Обновление выполнено успешно. Необходимо перезапустить приложение.");

    db->close();

}

void DialogUpdateDb::addLog(QString text)
{
    ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + text);
}

bool DialogUpdateDb::checkConnection()
{
    addLog("Подключаемся к БД...");
    db->close();
    db->setUserName(ui->lineEditName->text());
    db->setPassword(ui->lineEditPassword->text());
    if( db->open() )
    {
        addLog("ok\n");
        return true;
    }
    else
    {
        addLog("неудача.\n");
        return false;
    }
}

bool DialogUpdateDb::checkVersion()
{
    addLog("Проверка версии БД...");
    QSqlQuery query(*db);
    query.prepare("select svalue from appconfig where option='dbver';");
    if ( !query.exec() )
    {
        addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
        return false;
    }
    if (!query.next())
    {
        addLog("\nОшибка запроса: версия не обнаружена.");
        return false;
    }
    if ( query.value("svalue").toString() != QString(ApplicationConfiguration::olddbVersion) )
    {
        addLog("\nОшибка запроса: некорректная версия: " + query.value("svalue").toString());
        return false;
    }
    addLog("версия: " + query.value("svalue").toString() + "\n");
    return true;
}

bool DialogUpdateDb::updateV12V2()
{
    QSqlQuery query(*db);
    db->transaction();

    addLog("Добавляем столбец deleted в таблицу direction_users ...");
    query.prepare("ALTER TABLE direction_users ADD deleted SMALLINT DEFAULT 0;");
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();

    addLog("Добавляем столбец can_request, can_recorded и num в таблицу spr_job ...");
    query.prepare("ALTER TABLE spr_job "
                  "ADD can_request SMALLINT DEFAULT 0 NOT NULL, "
                  "ADD can_recorded SMALLINT DEFAULT 0 NOT NULL, "
                  "ADD num INTEGER DEFAULT 0 NOT NULL;");
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();

    addLog("Обновляем версию БД...");
    query.prepare("UPDATE appconfig SET svalue=:svalue where option='dbver';");
    query.bindValue(":svalue",ApplicationConfiguration::dbVersion);
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();


    addLog("Проверка текущий версии БД...");
    query.prepare("select svalue from appconfig where option='dbver';");
    if ( !query.exec() )
    {
        addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
        db->rollback();
        return false;
    }
    if (!query.next())
    {
        addLog("\nОшибка запроса: версия не обнаружена.");
        db->rollback();
        return false;
    }
    if ( query.value("svalue").toString() != QString(ApplicationConfiguration::dbVersion) )
    {
        addLog("\nОшибка запроса: некорректная версия: " + query.value("svalue").toString());
        db->rollback();
        return false;
    }
    addLog("версия: " + query.value("svalue").toString() + "\n");

    db->commit();

    db->transaction();

    addLog("Заполняем столбец deleted данными по умолчанию ...");
    query.prepare("UPDATE direction_users SET deleted=0;");
    if ( !query.exec() )
    {
        addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
        db->rollback();
        return false;
    }
    addLog("ok\n");
    query.clear();

    addLog("Обновляем столбец can_request данными по умолчанию ...");
    query.prepare("UPDATE spr_job SET can_request=0;");
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();

    addLog("Обновляем столбец can_recorded данными по умолчанию ...");
    query.prepare("UPDATE spr_job SET can_recorded=0;");
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();

    addLog("Обновляем столбец num данными по умолчанию ...");
    query.prepare("UPDATE spr_job SET num=0;");
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();

    db->commit();

    ui->pushButtonUpdateDB->setEnabled(false);
    return true;
}

bool DialogUpdateDb::updateV2V3()
{
    QSqlQuery query(*db);
    db->transaction();

    addLog("Увеличиваем размер поля text в таблице directions ...");
    query.prepare("ALTER TABLE directions ALTER text TYPE VARCHAR(4096);");
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();

    addLog("Обновляем версию БД...");
    query.prepare("UPDATE appconfig SET svalue=:svalue where option='dbver';");
    query.bindValue(":svalue",ApplicationConfiguration::dbVersion);
    if ( !query.exec() )
    {
          addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
          db->rollback();
          return false;
    }
    addLog("ok\n");
    query.clear();


    addLog("Проверка текущий версии БД...");
    query.prepare("select svalue from appconfig where option='dbver';");
    if ( !query.exec() )
    {
        addLog("\nОшибка запроса:" + query.lastQuery() + "\n\n" + query.lastError().text());
        db->rollback();
        return false;
    }
    if (!query.next())
    {
        addLog("\nОшибка запроса: версия не обнаружена.");
        db->rollback();
        return false;
    }
    if ( query.value("svalue").toString() != QString(ApplicationConfiguration::dbVersion) )
    {
        addLog("\nОшибка запроса: некорректная версия: " + query.value("svalue").toString());
        db->rollback();
        return false;
    }
    addLog("версия: " + query.value("svalue").toString() + "\n");

    db->commit();

    ui->pushButtonUpdateDB->setEnabled(false);
    return true;
}
