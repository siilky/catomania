
#include "stdafx.h"

#include "boost/signal.hpp"
#include "boost/bind.hpp"

#include "game/ai/task.h"
#include "game/ai/tnpc.h"



TSelectTarget::TSelectTarget(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TSelectTarget::onStarted()
{
    game_->getPlayer().sEvent.connect(boost::bind(&TSelectTarget::onEvent, this, boost::placeholders::_1));

    const Player & p = game_->getPlayer();
    if (p.getTarget() == targetId)
    {
        return TASK_COMPLETED;
    }

    clientdata::FragmentGiSelectTarget *selectTarget = new clientdata::FragmentGiSelectTarget();
    selectTarget->id = targetId;
    clientdata::FragmentGameinfoSet fgs(selectTarget);
    if ( ! game_->getConnection()->send(&fgs))
    {
        return TASK_FAILED;
    }
    return TASK_RUNNING;
}

void TSelectTarget::onEvent(BaseEvent *event)
{
    if (event->type() != BaseEvent::TargetStatus)
    {
        return;
    }

    TargetStatusEvent *e = static_cast<TargetStatusEvent*>(event);
    if (e->isPresent() && e->wId() == targetId)
    {
        state_ = TASK_COMPLETED;
    }
    else if (!e->isPresent())
    {
        state_ = TASK_FAILED;
    }
}

void TSelectTarget::onStopped()
{
    game_->getPlayer().sEvent.disconnect(boost::bind(&TSelectTarget::onEvent, this, boost::placeholders::_1));
}


TOpenNpc::TOpenNpc(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TOpenNpc::onStarted()
{
    if (npcId != Player::INVALID_ID)
    {
        if (cNpcGreeting_.empty())
        {
            cNpcGreeting_ = game_->getConnection()->bindServerHandler
                (std::function<void (const serverdata::FragmentGiNpcGreeting *)>
                (std::bind(&TOpenNpc::onNpcGreeting, this, std::placeholders::_1)));
        }
    }
    else
    {
        Log("No ID");
        return TASK_FAILED;
    }

    clientdata::FragmentGiOpenNpc *openNpc = new clientdata::FragmentGiOpenNpc();
    openNpc->id = npcId;
    clientdata::FragmentGameinfoSet fgs(openNpc);
    if ( ! game_->getConnection()->send(&fgs))
    {
        return TASK_FAILED;
    }
    return TASK_RUNNING;
}

void TOpenNpc::onStopped()
{
    npcId = Player::INVALID_ID;
    game_->getConnection()->unbindHandler(cNpcGreeting_);
}

void TOpenNpc::onNpcGreeting(const serverdata::NpcGreetingImpl * f)
{
    if (state_ != TASK_RUNNING)
    {
        return;
    }

    if (npcId == f->id)
    {
        Log("Greeting: %08X", npcId);
        state_ = TASK_COMPLETED;
    }
}
