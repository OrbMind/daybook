#ifndef MAINDEF_H
#define MAINDEF_H



namespace ApplicationConfiguration
{

    const char * const dbVersion = "1";
    const char * const userDBA = "SYSDBA";
    const char * const passwordDBA = "masterkey";
    //const char * const appName = "daybook";
    const char * const connectionOptions = "lc_ctype=UTF8";
    const char * const fullNameApplication = "Журнал Административных Распоряжений";
    const char * const briefNameApplication = "Журнал АР";
    const int idnGuest = -1;

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
    const int edit = 2;
    //const int del = 3;
    //const int restore = 4;
    const int initiate = 5;
    bool userPermission(int actionId,int userPermission);
}
#endif // MAINDEF_H

