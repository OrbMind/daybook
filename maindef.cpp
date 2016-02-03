#include "maindef.h"

bool Act::userPermission(int actionId,int userPermission)
{
    bool result = false;

    switch ( actionId )
    {
        //case Act::add:
        //    result = ( userPermission == UserRights::admin || userPermission == UserRights::writer );
        //break;
        //case Act::edit:
        //    result = ( userPermission == UserRights::admin || userPermission == UserRights::writer );
        //break;
        case Act::editDirection:
              result = ( userPermission == UserRights::admin || userPermission == UserRights::writer );
        break;
        case Act::editJob:
              result = ( userPermission == UserRights::admin );
        break;
        case Act::editUser:
              result = ( userPermission == UserRights::admin );
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
        break;
        case Act::showAdminInfo:
            result = ( userPermission == UserRights::admin );
        break;
    }

    return result;
}
