#include "maindef.h"
/*
const double // определение статической константы

CostEstimate::FudgeFactor = 1.35; // класса – помещается в файл реализации

char * const ConfigDbConst::dbVersion = "1";
*/

bool Act::userPermission(int actionId,int userPermission)
{
    bool result = false;

    switch ( actionId )
    {
        //case Act::add:
        //    result = ( userPermission == UserRights::admin || userPermission == UserRights::writer );
        //break;
        case Act::edit:
            result = ( userPermission == UserRights::admin || userPermission == UserRights::writer );
        break;
        //case Act::del:
        //    result = ( userPermission == UserRights::admin || userPermission == UserRights::writer );
        //break;
        case Act::read:
            result = ( userPermission == UserRights::admin || userPermission == UserRights::writer ||
                       userPermission == UserRights::user  || userPermission == UserRights::readonly);
        break;
        //case Act::restore:
        //    result = ( userPermission == UserRights::admin || userPermission == UserRights::writer );
        //break;
        case Act::initiate:
            result = ( userPermission == UserRights::admin || userPermission == UserRights::writer ||
                       userPermission == UserRights::user);
    }

    return result;
}
