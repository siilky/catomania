#include "stdafx.h"

#include "boost/signal.hpp"

#include "netdata/connection.h"
#include "netdata/fragments_client.h"
#include "game/account_types.h" 
#include "game/account.h" 
#include "error.h"
#include "event.h"

namespace Account
{

Account::Account()
{
    init();
}

Account::~Account()
{
    unbind();
}

void Account::bind(std::shared_ptr<Connection> connection)
{
    connection_ = connection;

    HANDLE_S(hooks_, connection_, Account, LoginIpInfo);
    HANDLE_S(hooks_, connection_, Account, AccInfo);
    HANDLE_S(hooks_, connection_, Account, CreateRoleRe);
    HANDLE_S(hooks_, connection_, Account, RoleListRe);
    HANDLE_C(hooks_, connection_, Account, SelectRole);
    HANDLE_S(hooks_, connection_, Account, SelectRoleRe);
    HANDLE_S(hooks_, connection_, Account, PlayerLogout);
}

void Account::unbind()
{
    if (connection_.get())
    {
        connection_->unbindHandlers(hooks_);
        connection_.reset();
    }

    init();
}

void Account::init()
{
    lastLoginTs_ = 0;
    lastIp_ = 0;
    currentIp_ = 0;

    accountId_ = INVALID_ID;
    currentChar_ = INVALID_ID;
    lastCharSelection_ = INVALID_ID;
    autoEnterWorld_ = false;
    charList_.resize(CHARLIST_SIZE);
    fill(charList_.begin(), charList_.end(), CharInfo());
}

bool Account::getAccountId(DWORD & accountId) const
{
    if (accountId_ != INVALID_ID)
    {
        accountId = accountId_;
        return true;
    }
    return false;
}

CharList Account::getCharList() const
{
    // remove empty slots
    CharList newList = charList_;
    for (CharList::iterator it = newList.begin(); it != newList.end();)
    {
        if (it->id == INVALID_ID)
        {
            it = newList.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return newList;
}

bool Account::selectChar(const std::wstring & name)
{
    auto iChar = std::find_if(charList_.begin(), charList_.end(),
                              [name](const CharInfo & c) { return c.name == name; });
    if (iChar == charList_.end())
    {
        return false;
    }

    Log("Selecting character %ls", iChar->name.c_str());

    autoEnterWorld_ = true;

    clientdata::FragmentSelectRole pSelectRole;
    pSelectRole.charId = iChar->id;
    pSelectRole.unk = 0;

    return connection_->send(&pSelectRole);
}

bool Account::getCurrentChar(CharInfo & charinfo) const
{
    if (currentChar_ != INVALID_ID
        && (size_t)currentChar_ < charList_.size())
    {
        charinfo = charList_[currentChar_];
        assert(charinfo.id != INVALID_ID);
        return true;
    }
    return false;
}

void Account::setIngameCharId(DWORD charId)
{
    charList_.resize(1);
    charList_[0] = CharInfo(charId, L"", CharacterRecord::CharacterStatusActive);
    currentChar_ = 0;
}

void Account::setIngameCharName(const std::wstring & charName)
{
    assert(currentChar_ == 0);
    charList_[0].name = charName;
}

void Account::getIpInfo(DWORD & lastLoginTs, DWORD & lastIp, DWORD & currentIp) const
{
    lastLoginTs = lastLoginTs_;
    lastIp      = lastIp_;
    currentIp   = currentIp_;
}

void Account::createCharacter(const std::wstring & name)
{
    if ( ! connection_)
    {
        return;
    }

    clientdata::FragmentCreatRole f;
    f.accId = accountId_;
    f.unk1 = 0;
    f.unk2 = 0;
    f.gender = CharacterGenderFemale;
    f.prof = CharacterClassCleric;
    f.level = 1;
    f.unk3 = 0;
    f.name = name;
    static const byte facedata[] = {
        0x01, 0x70, 0x00, 0x10, 0x72, 0x82, 0x80, 0x00, 0x17, 0x00, 0x17, 0x00,
        0x32, 0x75, 0x84, 0x00, 0x05, 0x00, 0x71, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x6B, 0x90, 0x80, 0x80, 0x80, 0x80, 0x80, 0x24, 0x00,
        0x0B, 0x00, 0x32, 0x02, 0x09, 0x00, 0x80, 0x88, 0x7A, 0x80, 0x80, 0x80, 0x7F, 0x80, 0x88, 0x7A,
        0x80, 0x80, 0x80, 0x7F, 0x07, 0x00, 0x1C, 0x00, 0x60, 0x5E, 0x77, 0x86, 0x6A, 0x80, 0x60, 0x5E,
        0x77, 0x86, 0x6A, 0x80, 0x01, 0x00, 0x10, 0x00, 0x82, 0x8C, 0x80, 0x91, 0x0F, 0x00, 0x80, 0x88,
        0x59, 0x00, 0x56, 0x00, 0x5C, 0x00, 0x79, 0x56, 0x7A, 0x80, 0x0E, 0x00, 0x57, 0x57, 0x83, 0x83,
        0x16, 0x00, 0x80, 0x80, 0x2B, 0x01, 0x38, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB2, 0x01,
        0xB4, 0x01, 0x00, 0x00, 0xFD, 0xFE, 0xFE, 0xFF, 0xEE, 0xD3, 0xED, 0xFF, 0x96, 0x96, 0x96, 0xFF,
        0xAA, 0xB2, 0xE7, 0xFF, 0xEA, 0xF2, 0xF2, 0xFF, 0x77, 0x78, 0x78, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0xFE, 0xFE, 0xFF, 0x7B, 0x7C, 0x75, 0x7A,
        0x7B, 0x74, 0x00, 0x00 };

    memset(&f.charRecord, sizeof(f.charRecord), 0);
    f.charRecord.faceData = barray(facedata, facedata + sizeof(facedata));
    f.charRecord.equip.resize(0x26);
    for (size_t i = 0; i < f.charRecord.equip.size(); i++)
    {
        CharacterRecord::EquipItem & item = f.charRecord.equip[i];
        item.pos = (DWORD)-1;
    }

    f.referrerRole = 0;
    f.cashAdd = 0;

    connection_->send(&f);
}

//

void Account::enterWorld()
{
    if (currentChar_ < 0 || currentChar_ > charList_.size())
    {
        Log("Failed to enter world - unknown char");
        return;
    }

    unsigned charId = charList_[currentChar_].id;

    Log("EnterWorld with %08X", charId);

    clientdata::FragmentEnterWorld pEnterWorld;
    pEnterWorld.charId = charId;

    connection_->send(&pEnterWorld);
}

//

void Account::onLoginIpInfo(const serverdata::FragmentLoginIpInfo *f)
{
    assert(f->accId == accountId_);

    lastLoginTs_ = f->lastLoginTs;
    lastIp_      = f->lastIp;
    currentIp_   = f->currentIp;
}

void Account::onAccInfo(const serverdata::FragmentAccInfo *f)
{
    charList_.resize(CHARLIST_SIZE);
    fill(charList_.begin(), charList_.end(), CharInfo());
    currentChar_ = INVALID_ID;
    accountId_ = f->accId;
}

void Account::onCreateRoleRe(const serverdata::FragmentCreateRoleRe * f)
{
    Log("CreateRoleRe res %i char %08X", f->result, f->charId);

    if (f->result != 0)
    {
        ServerErrorEvent e(f->result + 10150, L"", ServerError::getString(f->result + 10150));
        Log("Server Error: %ls", e.message().c_str());
        sEvent(&e);
        return;
    }

    // новый чар похоже помещается в первый свободный слот т к информация о слоте отсутствует в запросе и в ответе

    for (size_t i = 0; i < charList_.size(); i++)
    {
        if (charList_[i].id == INVALID_ID)  // TODO check status?
        {
            Log("New char added to slot: %i", i);
            charList_[i] = CharInfo(f->charId, f->name, f->charRecord.status);
            CharacterCreatedEvent e(f->charId, f->name);
            sEvent(&e);
            return;
        }
    }

    // failed to add char
    ServerErrorEvent e(10191, L"", ServerError::getString(10191));
    Log("Server Error: Failed to add new character");
    sEvent(&e);
}

void Account::onRoleListRe(const serverdata::FragmentRoleListRe *f)
{
    Log("Got %i slot with char %08X (%ls)", f->slot, f->charId, f->name.c_str());

    if (f->slot == 0xFFFFFFFF)
    {
        // no more slots, need selection
        CharSelectRequestEvent e;
        sEvent(&e);
        return;
    }

    if (accountId_ == INVALID_ID)
    {
        accountId_ = f->accId;
    }
    else if (accountId_ != f->accId)
    {
        Log("Invalid account ID state: have %08X received: %08X", accountId_, f->accId);
        assert(0);
        return;
    }

    if (f->slot > charList_.size())
    {
        Log("Incorrect slot number received (%i)", f->slot);
        assert(0);
        return;
    }

    charList_[f->slot] = CharInfo(f->charId, f->name, f->charRecord.status);
}

void Account::onSelectRole(const clientdata::FragmentSelectRole *f)
{
    lastCharSelection_ = f->charId;
    assert(lastCharSelection_ != INVALID_ID);
}

void Account::onSelectRoleRe(const serverdata::FragmentSelectRoleRe *f)
{
    if (f->code == serverdata::FragmentSelectRoleRe::Success)
    {
        if (lastCharSelection_ != INVALID_ID)
        {
            for (CharList::const_iterator charInfo = charList_.begin(); charInfo != charList_.end(); ++charInfo)
            {
                if (charInfo->id == lastCharSelection_)
                {
                    currentChar_ = charInfo - charList_.begin();
                    break;
                }
            }
        }

        CharselectCompletedEvent e;
        sEvent(&e);

        if (autoEnterWorld_)
        {
            enterWorld();
        }
    }
}

void Account::onPlayerLogout(const serverdata::FragmentPlayerLogout *f)
{
    if (f->type == PlayerLogoutCharSelect)
    {
        init();
    }
}

} // namespace
