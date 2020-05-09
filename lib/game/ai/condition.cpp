// $Id: condition.cpp 1777 2020-03-17 13:45:59Z jerry $
//

#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/ai/condition.h"
#include "game/game.h"

bool GotServerErrCond::operator ()(std::shared_ptr<Game> game) const
{
    int errorCode = game->getLastServerError();
    if (errorCode >= 0
        && errorCode != 45  // skip character creating errors: name already in use
        && errorCode != 25  // cannot use this name
        )
    {
        Log("Server Error: %i", errorCode);
        return true;
    }
    return false;
}

bool GotGameErrCond::operator ()(std::shared_ptr<Game> game) const
{
    int errorCode = game->getLastGameError();
    if (errorCode >= 0)
    {
        Log("Game Error (c): %i", errorCode);
        game->clearLastGameError();
        return true;
    }
    return false;
}
