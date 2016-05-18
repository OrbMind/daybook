#ifndef MAINDEF_H
#define MAINDEF_H

#include <QtCore>

namespace ApplicationConfiguration
{
    const char * const defaultHost = "10.0.2.200";
    const char * const defaultDbName = "test";
    const char * const olddbVersion = "2";
    const char * const dbVersion = "3";
    const char * const userDBA = "SYSDBA";
    const char * const passwordDBA = "masterkey";
    //const char * const appName = "daybook";
    const char * const connectionOptions = "lc_ctype=UTF8";
    const char * const fullNameApplication = "Журнал Административных Распоряжений";
    const char * const briefNameApplication = "Журнал АР";
    const int idnGuest = -1;
    //const char * const appVersion = APP_VERSION;

}

namespace UserRights
{
    const int readonly = -1;// только просмотр (Гость)
    const int user = 0; // просмотр и ознакомление
    const int writer = 1; // запись распоряжений
    const int admin = 2; // sudo
}

namespace Act
{
    const int read = 0;
    //const int add = 1;
    //const int edit = 2;
    //const int del = 3;
    //const int restore = 4;
    const int initiate = 5;
    const int showAdminInfo = 6;
    const int editDirection = 7;
    const int editJob = 8;
    const int editUser = 9;
    bool userPermission(int actionId,int userPermission);
}

namespace DataRole
{
    const int idn = Qt::UserRole;
    const int deleted = Qt::UserRole + 1;
}

#endif // MAINDEF_H

