
#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/ai/task.h"
#include "game/ai/tshop.h"


#define FTRACE() Log("*");
//#define TRACE()


TMarketSkill::TMarketSkill(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TMarketSkill::onStarted()
{
    FTRACE();

    if (cMarketSkill_.empty())
    {
        cMarketSkill_ = game_->getConnection()->bindServerHandler(
            std::function<void (const serverdata::FragmentGiPersonalMarketAvailable *)>(
            std::bind(&TMarketSkill::onMarketSkill, this, _1) ));
    }
    if (cErrorMsg_.empty())
    {
        cErrorMsg_ = game_->getConnection()->bindServerHandler(
            std::function<void (const serverdata::FragmentGiErrorMsg *)>(
            std::bind(&TMarketSkill::onGameError, this, _1) ));
    }

    clientdata::FragmentGiMarketSkill *fMarketSkill = new clientdata::FragmentGiMarketSkill;

    clientdata::FragmentGameinfoSet fgi(fMarketSkill);
    if ( ! game_->getConnection()->send(&fgi))
    {
        return TASK_FAILED;
    }

    return TASK_RUNNING;
}

void TMarketSkill::onStopped()
{
    FTRACE();
    game_->getConnection()->unbindHandler(cMarketSkill_);
    game_->getConnection()->unbindHandler(cErrorMsg_);
}

void TMarketSkill::onMarketSkill(const serverdata::FragmentGiPersonalMarketAvailable *)
{
    FTRACE();

    state_ = TASK_COMPLETED;
}

void TMarketSkill::onGameError(const serverdata::FragmentGiErrorMsg *)
{
    FTRACE();

    state_ = TASK_FAILED;
}


TOpenMarket::TOpenMarket(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TOpenMarket::onStarted()
{
    FTRACE();

    if (cOpenMarket_.empty())
    {
        cOpenMarket_ = game_->getConnection()->bindServerHandler(
            std::function<void (const serverdata::FragmentGiSelfOpenMarket *)>(
            std::bind(&TOpenMarket::onOpenMarket, this, _1) ));
    }

    clientdata::FragmentGiOpenMarket *fOpenMarket = new clientdata::FragmentGiOpenMarket;
    wcscpy_s(fOpenMarket->name, name.c_str());
    fOpenMarket->items = items;

    clientdata::FragmentGameinfoSet fgi(fOpenMarket);
    if ( ! game_->getConnection()->send(&fgi))
    {
        state_ = TASK_FAILED;
    }

    return TASK_RUNNING;
}

void TOpenMarket::onStopped()
{
    FTRACE();

    game_->getConnection()->unbindHandler(cOpenMarket_);
    game_->getConnection()->unbindHandler(cCloseMarket_);
}

void TOpenMarket::onOpenMarket(const serverdata::FragmentGiSelfOpenMarket *f)
{
    FTRACE();

    game_->getConnection()->unbindHandler(cOpenMarket_);

    // validate opened market

    bool validated = false;

    if (f->items.size() == items.size())
    {
        validated = true;

        for (size_t i = 0; i < f->items.size(); i++)
        {
            const serverdata::FragmentGiSelfOpenMarket::Item & ii = f->items[i];
            const MarketItem & mi = items[i];

            if (ii.id != mi.id
                || ii.count != mi.count
                || ii.slot != mi.slot
                || ii.price != mi.price)
            {
                Log("Item mismatch: at %i", i);
                validated = false;
                break;
            }
        }
    }
    else
    {
        Log("Item count mismatch: %i != %I", items.size(), f->items.size());
    }

    if (validated)
    {
        state_ = TASK_COMPLETED;
    }
    else
    {
        // send cancel and bail out

        Log("Cancelling market");

        if (cCloseMarket_.empty())
        {
            cCloseMarket_ = game_->getConnection()->bindServerHandler(
                std::function<void (const serverdata::FragmentGiPlayerCancelMarket *)>(
                std::bind(&TOpenMarket::onCloseMarket, this, _1) ));
        }

        clientdata::FragmentGiCancelMarket *fCancelMarket = new clientdata::FragmentGiCancelMarket;
        clientdata::FragmentGameinfoSet fgi(fCancelMarket);

        if ( ! game_->getConnection()->send(&fgi))
        {
            state_ = TASK_FAILED;
        }
    }
}

void TOpenMarket::onCloseMarket(const serverdata::FragmentGiPlayerCancelMarket *f)
{
    FTRACE();

    if (f->id == game_->getPlayer().getMyId())
    {
        game_->getConnection()->unbindHandler(cCloseMarket_);
        state_ = TASK_FAILED;
    }
}

//

TCloseMarket::TCloseMarket(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TCloseMarket::onStarted()
{
    Log("Closing shop");

    if ( ! game_->getPlayer().getAttrs().isCat())
    {
        Log("Game state reports already out of market");
        return TASK_COMPLETED;
    }

    if (cCloseMarket_.empty())
    {
        cCloseMarket_ = game_->getConnection()->bindServerHandler(
            std::function<void (const serverdata::FragmentGiPlayerCancelMarket *)>(
            std::bind(&TCloseMarket::onCloseMarket, this, _1) ));
    }

    clientdata::FragmentGiCancelMarket *fCancelMarket = new clientdata::FragmentGiCancelMarket;
    clientdata::FragmentGameinfoSet fgs(fCancelMarket);

    if ( ! game_->getConnection()->send(&fgs))
    {
        return TASK_FAILED;
    }
    return TASK_RUNNING;
}

void TCloseMarket::onStopped()
{
    FTRACE();
    game_->getConnection()->unbindHandler(cCloseMarket_);
}

void TCloseMarket::onCloseMarket(const serverdata::FragmentGiPlayerCancelMarket *f)
{
    FTRACE();

    if (f->id == game_->getPlayer().getMyId())
    {
        game_->getConnection()->unbindHandler(cCloseMarket_);
        state_ = TASK_COMPLETED;
    }
}


TOpenPlayerMarket::TOpenPlayerMarket(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TOpenPlayerMarket::onStarted()
{
    FTRACE();

    if (game_->getPlayer().getAttrs().isCat())
    {
        Log("Game state reports already in market");
        return TASK_COMPLETED;
    }

    if (cMarketInfo_.empty())
    {
        cMarketInfo_ = game_->getConnection()->bindServerHandler(
            std::function<void (const serverdata::FragmentGiPlayerMarketInfo *)>(
            std::bind(&TOpenPlayerMarket::onMarketInfo, this, _1) ));
    }

    clientdata::FragmentGiNpcCommand *npcCommand = new clientdata::FragmentGiNpcCommand();
    npcCommand->cmd = 0x13;
    clientdata::FragmentGameinfoSet fgs(npcCommand);
    if ( ! game_->getConnection()->send(&fgs))
    {
        return TASK_FAILED;
    }
    return TASK_RUNNING;
}

void TOpenPlayerMarket::onStopped()
{
    FTRACE();
    game_->getConnection()->unbindHandler(cMarketInfo_);
}

void TOpenPlayerMarket::onMarketInfo(const serverdata::FragmentGiPlayerMarketInfo *f)
{
    FTRACE();

    chk         = f->chk;
    characterID = f->characterID;
    items       = f->items;
    state_      = TASK_COMPLETED;
}
