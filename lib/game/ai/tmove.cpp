
#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/ai/task.h"
#include "game/ai/tmove.h"

//#define TRACE       Log
#define TRACE


TMove::TMove(std::shared_ptr<Game> game)
    : game_(game)
    , lastMoveStamp_(0)
    , isMoving_(false)
{
}

ITask::State TMove::tick_(Timestamp timestamp)
{
    if (lastMoveStamp_ == 0)
    {
        // starting to move
        lastMoveStamp_ = timestamp;
        isMoving_ = true;

        TRACE("Starting move to %.2f %.2f %.2f", destination.x(), destination.y(), destination.z());
        return TASK_RUNNING;
    }
    else
    {
        return sendMove(timestamp);
    }
}

ITask::State TMove::onStarted()
{
    if (destination.isNull())
    {
        assert(0);
        return TASK_FAILED;
    }

    CoordEx currentPos = game_->getPlayer().getPosition();
    if (currentPos.equalsTo(destination))
    {
        Log("Would not move - already at destination");
        return TASK_COMPLETED;
    }

    if (cTracePos_.empty())
    {
        cTracePos_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentGiSelfTraceCurPos *)>
            (std::bind(&TMove::onSelfTraceCurPos, this, _1) ));
    }

    traceCtr_ = 0;
    lastMoveStamp_ = 0;
    isMoving_ = false;

    return TASK_RUNNING;
}

void TMove::onStopped()
{
    TRACE("Stopped move");
    if (isMoving_)
    {
        // cancel move
        Log("Canceling move");
        sendMove(lastMoveStamp_ + 50, true);
    }

    game_->getConnection()->unbindHandler(cTracePos_);
}

ITask::State TMove::sendMove(Timestamp timestamp, bool forceStop /*= false*/)
{
    // Клиент посылает пакет на передвижение по факту уже пройденного расстояния:
    // MoveStop отправляется в конце движения, moveStart по тикам.

    Player & p = game_->getPlayer();
    const Player::StatInfo &stats = p.getStatInfo();

    Timestamp td = timestamp - lastMoveStamp_;

    // moved since last tick
    float speed = p.isFlying() ? stats.flySpeed : stats.walkSpeed;
    float distDelta = speed /*Internal coordinates*/ * (float(td) / 1000)/*ms*/;

    CoordEx currentPos = p.getPosition();
    byte orientation = currentPos.directionTo(destination);
    TRACE("Current pos %.2f %.2f %.2f delta %.2f dir %i"
        , currentPos.x(), currentPos.y(), currentPos.z(), distDelta, orientation);

    // move currentPos
    float actual = currentPos.moveTo(destination, distDelta);

    if (currentPos.equalsTo(destination) || forceStop)
    {
        // send move stop

        clientdata::FragmentGiMoveStop * f = new clientdata::FragmentGiMoveStop;
        f->pos = destination;
        f->orientation = orientation;
        f->moveType = byte(p.getMoveType());
        f->seqNum = WORD(p.getMoveCounter());
        f->speed = WORD(speed * 0x100 + 0.5f);
        f->time = WORD(actual / speed * 1000);      // actual time delta fixes time spans

        clientdata::FragmentGameinfoSet fg(f);
        game_->getConnection()->send(&fg);

        TRACE("MoveStop: [%.2f %.2f %.2f] speed %.2f time %lli (%i) ctr %i"
            , destination.x(), destination.y(), destination.z()
            , speed, td, f->time, f->seqNum);

        isMoving_ = false;
        return TASK_COMPLETED;
    }
    else if (timestamp > (lastMoveStamp_ + MoveTickInterval))
    {
        // send next move tick

        clientdata::FragmentGiMove *f = new clientdata::FragmentGiMove;
        f->posFrom = currentPos/*p.getPosition() that's wrong*/;
        f->posTo = currentPos;
        f->time = WORD(td);
        f->speed = WORD(speed * 0x100 + 0.5f);
        f->moveType = byte(p.getMoveType());
        f->seqNum = WORD(p.getMoveCounter());

        clientdata::FragmentGameinfoSet fg(f);
        game_->getConnection()->send(&fg);

        TRACE("MoveTick: [%.2f %.2f %.2f] => [%.2f %.2f %.2f] speed %f time %lli ctr %i"
            , f->posFrom.x(), f->posFrom.y(), f->posFrom.z()
            , currentPos.x(), currentPos.y(), currentPos.z()
            , speed, td, f->seqNum);

        lastMoveStamp_ = timestamp;
    }

    return TASK_RUNNING;
}

void TMove::onSelfTraceCurPos(const serverdata::FragmentGiSelfTraceCurPos * f)
{
    if ( ! isMoving_)
    {
        return;
    }

    traceCtr_ ++;
    Log("Move TraceCurPos: [%.2f %.2f %.2f] trace %i ", f->pos.x(), f->pos.y(), f->pos.z(), traceCtr_);
    if (traceCtr_ > TracePosMax)
    {
        state_ = TASK_FAILED;
    }
}
