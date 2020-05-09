#include "stdafx.h"

#include "boost/signals.hpp"

#include "log.h"
#include "game/chat.h"
#include "game/gtime.h"


//#define FTRACE() Log("->")
#define FTRACE()


Chat::Chat()
{
    init();
}

Chat::~Chat()
{
    unbind();
}

void Chat::init()
{
    myId_ = INVALID_ID;
    myName_.clear();
}

void Chat::bind(std::shared_ptr<Connection> connection)
{
    if (connection_.get() != NULL)
    {
        unbind();
    }

    connection_ = connection;

    HANDLE_C(cookies_, connection_, Chat, EnterWorld);

    HANDLE_S(cookies_, connection_, Chat, WorldChat);
    HANDLE_S(cookies_, connection_, Chat, ChatMessage);
    HANDLE_S(cookies_, connection_, Chat, PrivateChat);
    HANDLE_S(cookies_, connection_, Chat, FactionChat);
}

void Chat::unbind()
{
    if (connection_.get() != NULL)
    {
        connection_->unbindHandlers(cookies_);
        connection_.reset();
    }

    init();
}

void Chat::setMyName(const std::wstring & name)
{
    myName_ = name;
}

//

void Chat::sendPrivateMessage(const std::wstring & playerName, DWORD playerId, const std::wstring & text)
{
    if (!connection_)   return;

    clientdata::FragmentPrivateChat f;
    f.type = clientdata::FragmentPrivateChat::ChatTypePrivate;
    f.myId = myId_;
    f.myName = myName_;
    f.toId = playerId;
    f.toName = playerName;
    f.text = text;
#if PW_SERVER_VERSION >= 1420   // 1422 ?
    f.u1 = 0;
#endif
    connection_->send(&f);
}

void Chat::sendChatMessage(ChatType type, const std::wstring & text)
{
    if (!connection_)   return;

    clientdata::FragmentChatMessage f;

    if (type == ChatTypeGroup)       { f.type = clientdata::FragmentChatMessage::ChatTypeGroup; }
    else if (type == ChatTypePublic) { f.type = clientdata::FragmentChatMessage::ChatTypePublic; }
    else                               f.type = clientdata::FragmentChatMessage::ChatTypePublic;

    f.u0 = 0;
    f.myId = myId_;
    f.u1 = 0;
    f.message = text;

    connection_->send(&f);
}

void Chat::sendGuildMessage(const std::wstring & text)
{
    if (!connection_)   return;

    clientdata::FragmentFactionChat f;
    f.u1 = 0;
    f.u2 = 0;
    f.myId = myId_;
    f.text = text;

    connection_->send(&f);
}

//

void Chat::onEnterWorld(const clientdata::FragmentEnterWorld * f)
{
    myId_ = f->charId;
}

void Chat::onWorldChat(const serverdata::FragmentWorldChat * f)
{
    switch (f->type)
    {
        case serverdata::FragmentWorldChat::ChatTypeWorld:
        case serverdata::FragmentWorldChat::ChatTypeHorn:
        {
            ChatMessageEvent::ChatType type =
                f->type == serverdata::FragmentWorldChat::ChatTypeWorld ? ChatMessageEvent::World
                : (f->type == serverdata::FragmentWorldChat::ChatTypeHorn ? ChatMessageEvent::Horn
                   : ChatMessageEvent::Unknown);
            ChatMessageEvent e( type
                              , f->plaintext.charId
                              , f->plaintext.charName
                              , f->plaintext.message);
            sEvent(&e);
            break;
        }

        case 0x08:
        {
            if (f->subType == 0x05)
            {
                // tw bets
                TWBetMessageEvent e(f->twBid.zoneNum, f->twBid.clanName, GTime::unixToTs(f->twBid.closingTime));
                sEvent(&e);
            }
            else
            {
                // VSB/GM Announce
            }

            break;
        }
    }
}

void Chat::onChatMessage(const serverdata::FragmentChatMessage *f)
{
    switch (f->type)
    {
        case serverdata::FragmentChatMessage::ChatTypePublic:
        case serverdata::FragmentChatMessage::ChatTypeGroup:
        {
            ChatMessageEvent::ChatType type =
                f->type == serverdata::FragmentChatMessage::ChatTypePublic ? ChatMessageEvent::Local
                : (f->type == serverdata::FragmentChatMessage::ChatTypeGroup ? ChatMessageEvent::Party
                   : ChatMessageEvent::Unknown);
            ChatMessageEvent e(type
                               , f->plaintext.id
                               , std::wstring()
                               , f->plaintext.message);
            sEvent(&e);
            break;
        }
    }
}

void Chat::onPrivateChat(const serverdata::FragmentPrivateChat *f)
{
    switch (f->type)
    {
        case serverdata::FragmentPrivateChat::ChatTypePrivate:
        {
            ChatMessageEvent e(ChatMessageEvent::Private
                               , f->fromId
                               , f->fromName
                               , f->text);
            sEvent(&e);
            break;
        }
    }
}

void Chat::onFactionChat(const serverdata::FragmentFactionChat * f)
{
    ChatMessageEvent e(ChatMessageEvent::Clan
                       , f->senderId
                       , std::wstring()
                       , f->text);
    sEvent(&e);
}
