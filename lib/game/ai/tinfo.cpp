
#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/game.h"
#include "game/ai/task.h"
#include "game/ai/tinfo.h"


#define FTRACE() Log("->");
//#define FTRACE()


ITask::State TGetFriendlist::onStarted()
{
    FTRACE();

    // send a friend list request so char will be marked as online in friendlists
    Account::CharInfo  currentChar;
    Account::Account & account = game_->getAccount();

    if (!account.getCurrentChar(currentChar))
    {
        Log("Charselect: No Char Selected");
        assert(0);
        return TASK_FAILED;
    }

    cFriendList_ = game_->getConnection()->bindServerHandler(
                    std::function<void (const serverdata::FragmentGetFriendsRe *)>(
                        std::bind(&TGetFriendlist::onFriendListRe, this, _1) ));

    clientdata::FragmentGetFriends pGetFriends;
    pGetFriends.myId = currentChar.id;
    pGetFriends.unk = 0;

    if (game_->getConnection()->send(&pGetFriends))
    {
        return TASK_RUNNING;
    }
    else
    {
        return TASK_FAILED;
    }
}

void TGetFriendlist::onStopped()
{
    game_->getConnection()->unbindHandler(cFriendList_);
}

void TGetFriendlist::onFriendListRe(const serverdata::FragmentGetFriendsRe * /*f*/)
{
    Log("Got friendlist");
    state_ = TASK_COMPLETED;
}
