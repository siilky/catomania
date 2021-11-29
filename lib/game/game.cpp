#include "stdafx.h"

#include "boost/signal.hpp"
#include "boost/bind.hpp"

#include "game/game.h"
#include "netdata/connection.h"


Game::Game()
    : lastGiError_(-1)
    , lastServerError_(-1)
    , gtimer_(new GTimer())
    , itemPropFactory_(new ItemPropsFactory())
    , player_(gtimer_)
    , world_(gtimer_)
    , gacEnabled_(false)
{
    account_  .sEvent.connect(boost::bind(boost::ref(sBaseEvent), boost::placeholders::_1));
    player_   .sEvent.connect(boost::bind(boost::ref(sBaseEvent), boost::placeholders::_1));
    inventory_.sEvent.connect(boost::bind(boost::ref(sBaseEvent), boost::placeholders::_1));
    world_    .sEvent.connect(boost::bind(boost::ref(sBaseEvent), boost::placeholders::_1));
    trade_    .sEvent.connect(boost::bind(boost::ref(sBaseEvent), boost::placeholders::_1));
    chat_     .sEvent.connect(boost::bind(boost::ref(sBaseEvent), boost::placeholders::_1));
    commi_    .sEvent.connect(boost::bind(boost::ref(sBaseEvent), boost::placeholders::_1));

    // opt: connecting only needed signal
    account_  .sEvent.connect(boost::bind(&Game::onBaseEvent, this, boost::placeholders::_1));
}

Game::~Game()
{
    unbind();
}

void Game::init()
{
    // initialize all game states to the 'initial' state
    account_.init();
    world_.init();
    player_.init();
    inventory_.init();
    chat_.init();
    trade_.init();
    gac_.init();
    gameTask_.init();
    commi_.init();

    clearLastGameError();
    clearLastServerError();
}

void Game::bind(std::shared_ptr<Connection> connection)
{
    connection_ = connection;
    world_.bind(connection_);
    player_.bind(connection_);
    account_.bind(connection_);
    inventory_.bind(connection_);
    chat_.bind(connection_);
    trade_.bind(connection_);
    commi_.bind(connection_);

    if (gacEnabled_)
    {
        gac_.bind(connection_);
    }
    gameTask_.bind(connection_);

#if PW_SERVER_VERSION >= 1700
    connection_->setGiIdEncodeVal2(elementsVersion_);
#endif

    HANDLE_S_GI(hooks_, connection_, Game, ErrorMsg);
    HANDLE_S(hooks_, connection_, Game, ErrorInfo);
    HANDLE_S(hooks_, connection_, Game, SelectRoleRe);
    HANDLE_S(hooks_, connection_, Game, BannedMessage);
}

void Game::unbind()
{
    world_.unbind();
    player_.unbind();
    account_.unbind();
    inventory_.unbind();
    chat_.unbind();
    trade_.unbind();
    gac_.unbind();
    gameTask_.unbind();
    commi_.unbind();

    if (connection_)
    {
        connection_->unbindHandlers(hooks_);
        connection_.reset();
    }
}

void Game::setElements(const elements::ItemListCollection & items)
{
    elementsVersion_ = items.version();

    inventory_.setElements(items);
    commi_.setElements(items);

    itemPropFactory_->addSet(ItemPropClass(items.allIds(elements::WEAPONS),     itemPropsStaticCtor<WeaponProps>));
    itemPropFactory_->addSet(ItemPropClass(items.allIds(elements::ARMORS),      itemPropsStaticCtor<ArmorProps>));
    itemPropFactory_->addSet(ItemPropClass(items.allIds(elements::ORNAMENTS),   itemPropsStaticCtor<ArmorProps/*fixme*/>));
    itemPropFactory_->addSet(ItemPropClass(items.allIds(elements::FASHION),     itemPropsStaticCtor<FashionProps>));
    itemPropFactory_->addSet(ItemPropClass(items.allIds(elements::FLY_MOUNTS),  itemPropsStaticCtor<FlyMountProps>));
    itemPropFactory_->addSet(ItemPropClass(items.allIds(elements::GENIES),      itemPropsStaticCtor<GenieProps>));

    inventory_.setPropFactory(itemPropFactory_);
    commi_.setPropFactory(itemPropFactory_);
}

void Game::onBaseEvent(BaseEvent *event)
{
    switch (event->type())
    {
        case BaseEvent::CharselectCompleted:
        {
            Account::CharInfo charinfo;
            if (account_.getCurrentChar(charinfo))
            {
                chat_.setMyName(charinfo.name);
            }
            break;
        }
    }
}

void Game::clearLastServerError()
{
    lastServerError_ = -1;
}

void Game::clearLastGameError()
{
    lastGiError_ = -1;
}

void Game::tick(Timestamp timestamp)
{
    world_.tick(timestamp);
    player_.tick(timestamp);
    gameTask_.tick(timestamp);
    gac_.tick(timestamp);
}

//

void Game::onErrorMsg(const serverdata::FragmentGiErrorMsg *f)
{
    lastGiError_ = f->id;

    GameErrorEvent e(ServerError::getString(f->id));
    sBaseEvent(&e);
    Log("Game Error: %ls", e.message().c_str());
}

void Game::onErrorInfo(const serverdata::FragmentErrorInfo *f)
{
    lastServerError_ = f->code;

    ServerErrorEvent e(f->code + 10150, strToWStr(f->baseMessage), ServerError::getString(f->code + 10150));
    sBaseEvent(&e);
    Log("Server Error: %ls", e.message().c_str());
}

void Game::onSelectRoleRe(const serverdata::FragmentSelectRoleRe *f)
{
    if (f->code == 0)
    {
        return;
    }

    lastServerError_ = f->code;

    ServerErrorEvent e(f->code + 10150, L"", ServerError::getString(f->code + 10150));
    sBaseEvent(&e);
    Log("Server Error: %ls", e.message().c_str());
}

void Game::onBannedMessage(const serverdata::FragmentBannedMessage *f)
{
    BanMessageEvent e(f->message, f->banTs, f->secsRemaining);
    sBaseEvent(&e);
    Log("Ban message: %ls", e.message().c_str());
}
