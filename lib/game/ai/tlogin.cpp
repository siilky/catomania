
#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/ai/task.h"
#include "game/ai/tlogin.h"
#include "game/ai/condition.h"
#include "netdata/connection.h"
#include "netdata/fragments_client.h"

// #define FTRACE() Log("*");
#define FTRACE()


TGetAccountCharlist::TGetAccountCharlist(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TGetAccountCharlist::onStarted()
{
    FTRACE();

    if (cRoleList.empty())
    {
        cRoleList = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentRoleListRe *)>
            (std::bind(&TGetAccountCharlist::onRoleList, this, _1) ));
    }

    if (getNextRole(-1))
    {
        return TASK_RUNNING;
    }
    else
    {
        return TASK_FAILED;
    }
}

void TGetAccountCharlist::onStopped()
{
    FTRACE();
    game_->getConnection()->unbindHandler(cRoleList);
}

ITask::State TGetAccountCharlist::onCompleted()
{
    FTRACE();
    return state_;
}

bool TGetAccountCharlist::getNextRole(int slot)
{
    Log("Getting slot: %d", slot);

    clientdata::FragmentRoleList f;

    bool result = game_->getAccount().getAccountId(f.accId);
    assert(result);

    f.unk = 0;
    f.slot = slot;

    return game_->getConnection()->send(&f);
}

void TGetAccountCharlist::onRoleList(const serverdata::FragmentRoleListRe *f)
{
    assert(state_ == TASK_RUNNING);

    if(f->slot != 0xFFFFFFFF)
    {
        // get next slot
        if ( ! getNextRole(f->slot))
        {
            state_ = TASK_FAILED;
        }
    }
    else
    {
        // No more slots - able to select character
        Log("No more slots");
        state_ = TASK_COMPLETED;
    }
}

//

TGetInventory::TGetInventory(std::shared_ptr<Game> game)
    : game_(game)
{
    stopWhen(GCondition<GotInventoryCond>(game_));
}

ITask::State TGetInventory::onStarted()
{
    FTRACE();

    clientdata::FragmentGiGetFullInfo *fGetFullInfo = new clientdata::FragmentGiGetFullInfo;
    fGetFullInfo->unk1 = 1;
    fGetFullInfo->unk2 = 1;
    fGetFullInfo->unk3 = 0;
    clientdata::FragmentGameinfoSet fgi(fGetFullInfo);

    if ( ! game_->getConnection()->send(&fgi))
    {
        return TASK_FAILED;
    }

    return TASK_RUNNING;
}

TPerformLogin::TPerformLogin(std::shared_ptr<Game> game)
    : game_(game)
{
    task_ =
        (
            task<TWait>(20000)
                .stopWhen(GCondition<GotAccountCond>(game))
            & task<TGetAccountCharlist>(game)
            & task<TWait>()
                .stopWhen(GCondition<CharSelectedCond>(game))
            & task<TWait>(40000)
                .stopWhen(GCondition<GotCharAttrs>(game) && GCondition<GotLockInfo>(game))
        ).failWhen(GCondition<GotServerErrCond>(game))
        ;
}

void TPerformLogin::onStopped()
{
    FTRACE();
}


TPerformIngameLogin::TPerformIngameLogin(std::shared_ptr<Game> game)
    : game_(game)
{
    task_ =
        (
            task<TWait>()
                .stopWhen(GCondition<CharSelectedCond>(game))
            & task<TWait>(60000)
                .stopWhen(GCondition<GotCharAttrs>(game) && GCondition<GotLockInfo>(game))
        ).failWhen(GCondition<GotServerErrCond>(game))
        ;
}
