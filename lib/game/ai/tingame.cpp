
#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/ai/task.h"
#include "game/ai/tingame.h"

#define FTRACE() Log("*");
//#define FTRACE()


TWaitKeepalive::TWaitKeepalive(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TWaitKeepalive::onStarted()
{
    FTRACE();

    if (cKeepalive_.empty())
    {
        cKeepalive_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentKeepalive *)>
            (std::bind(&TWaitKeepalive::onKeepalive, this, _1)));
    }

    return TASK_RUNNING;
}

void TWaitKeepalive::onStopped()
{
    FTRACE();

    game_->getConnection()->unbindHandler(cKeepalive_);
}

void TWaitKeepalive::onKeepalive(const serverdata::FragmentKeepalive * /*f*/)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    state_ = TASK_COMPLETED;
}


TGetMyId::TGetMyId(std::shared_ptr<Game> game)
    : TWait(10000)
    , game_(game)
{

}

ITask::State TGetMyId::onStarted()
{
    FTRACE();

    if (cEmotionRe_.empty())
    {
        cEmotionRe_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentGiDoEmotion *)>
            (std::bind(&TGetMyId::onEmotionRe, this, _1) ));
    }
    #pragma warning(suppress:28159)   // C28159: Consider using 'GetTickCount64' instead of 'GetTickCount'
    seed_ = (WORD)GetTickCount() | 0x7F;        // mask user emotions
    Log("Sending seed: %04X", seed_);

    clientdata::FragmentGiEmotion *fEmotion = new clientdata::FragmentGiEmotion;
    fEmotion->emotionId = seed_;
    clientdata::FragmentGameinfoSet fgi(fEmotion);

    game_->getConnection()->send(&fgi);

    return TASK_RUNNING;
}

void TGetMyId::onStopped()
{
    FTRACE();

    seed_ = 0;
    game_->getConnection()->unbindHandler(cEmotionRe_);
}

void TGetMyId::onEmotionRe(const serverdata::FragmentGiDoEmotion *f)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    Log("seed id %08X, %04X", f->charId, f->emotionId);

    if (f->emotionId != seed_)
    {
        state_ = TASK_FAILED;
        return;
    }

    game_->getAccount().setIngameCharId(f->charId);

    state_ = TASK_COMPLETED;
}


TGetMyAttrs::TGetMyAttrs(std::shared_ptr<Game> game)
    : game_(game)
{

}

ITask::State TGetMyAttrs::onStarted()
{
    FTRACE();

    Account::CharInfo me;
    if ( ! game_->getAccount().getCurrentChar(me))
    {
        return TASK_FAILED;
    }

    assert(cCharacters_.empty());

    if (cCharacters_.empty())
    {
        cCharacters_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentGiCharacters *)>
            (std::bind(&TGetMyAttrs::onCharAttrs, this, _1) ));
    }

    clientdata::FragmentGiGetCharAttrs *fGetCharAttrs = new clientdata::FragmentGiGetCharAttrs;
    fGetCharAttrs->characters.push_back(me.id);
    clientdata::FragmentGameinfoSet fgi(fGetCharAttrs);
    game_->getConnection()->send(&fgi);

    return TASK_RUNNING;
}

void TGetMyAttrs::onStopped()
{
    FTRACE();

    game_->getConnection()->unbindHandler(cCharacters_);
}

void TGetMyAttrs::onCharAttrs(const serverdata::FragmentGiCharacters *f)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    Account::CharInfo me;
    if ( ! game_->getAccount().getCurrentChar(me))
    {
        state_ = TASK_FAILED;
        return;
    }

    for (size_t i = 0; i < f->characters.size(); i++)
    {
        const struct serverdata::Character &c = f->characters[i];
        Log("id:%08X", c.id);

        if (c.id == me.id)
        {
            // ok done
            state_ = TASK_COMPLETED;
            break;
        }
    }
}

TGetMyInfo::TGetMyInfo(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TGetMyInfo::onStarted()
{
    FTRACE();
    assert(cBaseInfo_.empty());

    if (cBaseInfo_.empty())
    {
        cBaseInfo_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentPlayerBaseInfoRe *)>
            (std::bind(&TGetMyInfo::onBaseInfo, this, _1) ));
    }

    Account::CharInfo me;
    if ( ! game_->getAccount().getCurrentChar(me))
    {
        return TASK_FAILED;
    }

    clientdata::FragmentPlayerBaseInfo fGetBaseInfo;
    fGetBaseInfo.characters.push_back(me.id);
    fGetBaseInfo.myId = me.id;
    fGetBaseInfo.unk = 0;
    game_->getConnection()->send(&fGetBaseInfo);

    return TASK_RUNNING;
}

void TGetMyInfo::onStopped()
{
    game_->getConnection()->unbindHandler(cBaseInfo_);
}

void TGetMyInfo::onBaseInfo(const serverdata::FragmentPlayerBaseInfoRe *f)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    Account::CharInfo me;
    if ( ! game_->getAccount().getCurrentChar(me))
    {
        state_ = TASK_FAILED;
        return;
    }

    if (f->id == me.id)
    {
        game_->getAccount().setIngameCharName(f->charName);
        state_ = TASK_COMPLETED;
    }
}

TWaitEnterGame::TWaitEnterGame(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TWaitEnterGame::onStarted()
{
    FTRACE();

    if (cEnterGame_.empty())
    {
        cEnterGame_ = game_->getConnection()->bindClientHandler
            (std::function<void (const clientdata::FragmentEnterWorld *)>
            (std::bind(&TWaitEnterGame::onEnterGame, this, _1)));
    }

    return TASK_RUNNING;
}

void TWaitEnterGame::onStopped()
{
    FTRACE();

    game_->getConnection()->unbindHandler(cEnterGame_);
}

void TWaitEnterGame::onEnterGame(const clientdata::FragmentEnterWorld * /*f*/)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    state_ = TASK_COMPLETED;
}

TWaitPlayerRestart::TWaitPlayerRestart(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TWaitPlayerRestart::onStarted()
{
    FTRACE();

    if (cPlayerLogout_.empty())
    {
        cPlayerLogout_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentPlayerLogout *)>
            (std::bind(&TWaitPlayerRestart::onPlayerLogout, this, _1)));
    }

    if (cRoleList_.empty())
    {
        cRoleList_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentRoleListRe *)>
            (std::bind(&TWaitPlayerRestart::onRoleList, this, _1) ));
    }

    return TASK_RUNNING;
}

void TWaitPlayerRestart::onStopped()
{
    FTRACE();

    game_->getConnection()->unbindHandler(cPlayerLogout_).unbindHandler(cRoleList_);
}

void TWaitPlayerRestart::onPlayerLogout(const serverdata::FragmentPlayerLogout * /*f*/)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    state_ = TASK_COMPLETED;
}

void TWaitPlayerRestart::onRoleList(const serverdata::FragmentRoleListRe * /*f*/)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    state_ = TASK_COMPLETED;
}


TGetBattleMap::TGetBattleMap(std::shared_ptr<Game> game)
    : game_(game)
{
}

ITask::State TGetBattleMap::onStarted()
{
    FTRACE();
    assert(cTwMap_.empty());

    if (cTwMap_.empty())
    {
        cTwMap_ = game_->getConnection()->bindServerHandler
            (std::function<void (const serverdata::FragmentBattleGetMapRe *)>
            (std::bind(&TGetBattleMap::onBattleGetMapRe, this, _1) ));
    }

    Account::CharInfo me;
    if ( ! game_->getAccount().getCurrentChar(me))
    {
        return TASK_FAILED;
    }

    clientdata::FragmentBattleGetMap fBattleGetMap;
    fBattleGetMap.myId = me.id;
    fBattleGetMap.unk = 0;
    game_->getConnection()->send(&fBattleGetMap);

    return TASK_RUNNING;
}

void TGetBattleMap::onStopped()
{
    game_->getConnection()->unbindHandler(cTwMap_);
}

void TGetBattleMap::onBattleGetMapRe(const serverdata::FragmentBattleGetMapRe * /*f*/)
{
    FTRACE();
    assert(state_ == TASK_RUNNING);

    state_ = TASK_COMPLETED;
}
