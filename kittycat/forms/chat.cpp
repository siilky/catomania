#include "stdafx.h"

#include <array>

#include "netdata\content.h"
#include "chat.hpp"
#include "ui_chat.h"
#include "catctl.h"

static const QColor TextColorLocal(0xFF, 0xFF, 0xFF);   // ChatTypeLocal
static const QColor TextColorWorld(0xFF, 0xE4, 0x00);   // ChatTypeWorld
static const QColor TextColorHorn(0xFF, 0x9B, 0x3E);    //
static const QColor TextColorClan(0x00, 0xFF, 0xFF);    // ChatTypeClan
static const QColor TextColorGroup(0x00, 0xFF, 0x00);   // ChatTypeGroup
static const QColor TextColorPrivate(0x51, 0x96, 0xFF); // ChatTypePrivate
static const QColor TextColorSystem(0xA0, 0xA0, 0xA0);

//

inline static QString playerIdString(unsigned id)
{
    return QString("%1").arg(QString::number(id, 16).toUpper(), 8, '0');
}

inline static QString playerAnchorString(unsigned id, const QString & name)
{
    return QString("char:%1&%2")
        .arg(QString::number(id))
        .arg(name);
}

inline static QString playerNoNamedString(unsigned id)
{
    return QString("<span title=\"char_id\">%1</span>")
        .arg(playerIdString(id));
}


//

Chat::Chat(QSharedPointer<CatCtl> ctl, QWidget * parent)
    : QWidget(parent)
	, ui(new Ui::Chat())
    , ctl_(ctl)
{
	ui->setupUi(this); 

    // set types
    int filter;
    ctl_->config().get(L"ChatFilter", filter, -1);
    if (filter < 0)
    {
        filter_ = QFlags<ChatType>() | ChatTypeLocal | ChatTypeWorld | ChatTypeClan | ChatTypeGroup | ChatTypePrivate;
    }
    else
    {
        filter_ = QFlags<ChatType>(filter);
    }

    ui->tbPublic->setChecked(filter_.testFlag(ChatTypeLocal));
    ui->tbWorld->setChecked(filter_.testFlag(ChatTypeWorld));
    ui->tbClan->setChecked(filter_.testFlag(ChatTypeClan));
    ui->tbGroup->setChecked(filter_.testFlag(ChatTypeGroup));
    ui->tbPrivate->setChecked(filter_.testFlag(ChatTypePrivate));

    ui->cbType->addItem(tr("Private"), ChatTypePrivate);
    ui->cbType->addItem(tr("Guild"), ChatTypeClan);
    ui->cbType->addItem(tr("Party"), ChatTypeGroup);
    ui->cbType->setCurrentIndex(2);

    connect(ui->tbPublic,   &QToolButton::toggled, this, &Chat::updateFilters);
    connect(ui->tbWorld,    &QToolButton::toggled, this, &Chat::updateFilters);
    connect(ui->tbClan,     &QToolButton::toggled, this, &Chat::updateFilters);
    connect(ui->tbGroup,    &QToolButton::toggled, this, &Chat::updateFilters);
    connect(ui->tbPrivate,  &QToolButton::toggled, this, &Chat::updateFilters);
    connect(ui->tbText,     &QTextBrowser::anchorClicked, this, &Chat::onAnchorClicked);

    connect(ctl_.data(), &CatCtl::gameEvent, this, &Chat::onEvent);
    connect(ctl_.data(), &CatCtl::connected, this, &Chat::enableInput);
    connect(ctl_.data(), &CatCtl::disconnected, this, &Chat::disableInput);

//    disableInput();

    const Options & opts = ctl_->getOptions();
    setBufferSize(opts.chatBufferSize);
}

Chat::~Chat()
{
    ctl_->config().set(L"ChatFilter", static_cast<int>(filter_));
    delete ui;
}

void Chat::setBufferSize(int chatBufferOption)
{
    static const std::array<int, 5> sizes {
        30,     // tiny
        50,     // small
        100,    // medium
        200,    // large
        500,    // huge
    };
    if (chatBufferOption >= (int)sizes.size())
    {
        chatBufferOption = sizes.size() - 1;
    }

    maxBlocks_ = sizes[chatBufferOption];
    ui->tbText->document()->setMaximumBlockCount(maxBlocks_);
}

void Chat::setEnabled(bool isEnabled /*= true*/)
{
    ui->leInput->setEnabled(isEnabled);
}

//

void Chat::on_leInput_textChanged(const QString & text)
{
    if (text.size() < 2)
    {
        return;
    }

    int currentIndex = ui->cbType->currentData().toInt();
    if (text.startsWith('/') && currentIndex != ChatTypePrivate)
    {
        int needIndex = ui->cbType->findData(ChatTypePrivate);
        ui->cbType->setCurrentIndex(needIndex);
    }
    else if (text.startsWith("!~") && currentIndex != ChatTypeClan)
    {
        int needIndex = ui->cbType->findData(ChatTypeClan);
        ui->cbType->setCurrentIndex(needIndex);
        ui->leInput->setText(QString(text).remove(0, 2));
    }
    else if (text.startsWith("!!") && currentIndex != ChatTypeGroup)
    {
        int needIndex = ui->cbType->findData(ChatTypeGroup);
        ui->cbType->setCurrentIndex(needIndex);
        ui->leInput->setText(QString(text).remove(0, 2));
    }
}

void Chat::on_leInput_returnPressed()
{
    int type = ui->cbType->currentData().toInt();
    QString text = ui->leInput->text();

    if (text.isEmpty())
    {
        return;
    }

    switch (type)
    {
        case ChatTypeClan:
            ctl_->sendGuildMessage(text);
            // no text adding
            break;

        case ChatTypeGroup:
        {
            ctl_->sendGroupMessage(text);

            QString myName = ctl_->getMyName();
            ChatMessage msg = formatMessage(ChatTypeGroup, 0, myName, text, true);
            appendMessage(msg);

            break;
        }

        case ChatTypePrivate:
            if (text.startsWith('/'))
            {
                int i = text.indexOf(' ');
                if (i >= 0)
                {
                    QString nickName = text.left(i).remove(0, 1);
                    text = text.right(text.size() - i);

                    if (!nickName.isEmpty() && !text.isEmpty())
                    {
                        ctl_->sendPrivateMessage(nickName, text);

                        // append text 
                        ChatMessage msg = formatMessage(ChatTypePrivate, 0, nickName, text, true);
                        appendMessage(msg);

                        break;
                    }
                }
            }
            // else no 
            return;

        default:
            // no action
            return;
    }

    // if succeeded remove text
    ui->leInput->clear();
}

void Chat::disableInput()
{
    ui->cbType->setDisabled(true);
    ui->leInput->setDisabled(true);
}

void Chat::enableInput()
{
    ui->cbType->setEnabled(true);
    ui->leInput->setEnabled(true);
}

void Chat::updateFilters()
{
    if (ui->tbPublic->isChecked())  filter_ |= ChatTypeLocal; else filter_ &= ~ChatTypeLocal;
    if (ui->tbWorld->isChecked())   filter_ |= ChatTypeWorld; else filter_ &= ~ChatTypeWorld;
    if (ui->tbClan->isChecked())    filter_ |= ChatTypeClan; else filter_ &= ~ChatTypeClan;
    if (ui->tbGroup->isChecked())   filter_ |= ChatTypeGroup; else filter_ &= ~ChatTypeGroup;
    if (ui->tbPrivate->isChecked()) filter_ |= ChatTypePrivate; else filter_ &= ~ChatTypePrivate;

    updateChat();
}


void Chat::onEvent(const qlib::GameEvent & event)
{
    switch (event->type())
    {
        case BaseEvent::ChatMessage:
        {
            const ChatMessageEvent *e = static_cast<ChatMessageEvent*>(event.data());

            static const QMap<ChatMessageEvent::ChatType, ChatType> typeMap {
                { ChatMessageEvent::Local, ChatTypeLocal },
                { ChatMessageEvent::World, ChatTypeWorld },
                { ChatMessageEvent::Horn, ChatTypeHorn },
                { ChatMessageEvent::Clan, ChatTypeClan },
                { ChatMessageEvent::Party, ChatTypeGroup },
                { ChatMessageEvent::Trade, ChatTypeLocal },
                { ChatMessageEvent::Private, ChatTypePrivate },
            };

            ChatType type = typeMap.contains(e->type()) ? typeMap[e->type()] : ChatTypeLocal;

            ChatMessage msg = formatMessage(type, e->charId(), QString::fromStdWString(e->charName()), QString::fromStdWString(e->message()));
            appendMessage(msg);

            break;
        }

        case BaseEvent::PlayerResolved:
        {
            PlayerResolvedEvent *e = static_cast<PlayerResolvedEvent*>(event.data());
            playerResolved(e->gId(), QString::fromStdWString(e->name()));
            break;
        }

        case BaseEvent::StatusAnnounce:
        {
            StatusAnnounceEvent *e = static_cast<StatusAnnounceEvent*>(event.data());
            appendString(tr("<font color=#%1>%2 is offline</font>")
                .arg(QString::number(TextColorSystem.rgb(), 16))
                .arg(QString::fromStdWString(e->name())));
        }
    }
}

void Chat::onAnchorClicked(const QUrl & link)
{
    if (link.scheme() == "char")
    {
        QString name = link.path().section('&', 1);
        QString text = ui->leInput->text();

        if (text.startsWith('/'))
        {
            int pSep = text.indexOf(' ');
            if (pSep >= 0)
            {
                text.remove(0, pSep);
            }
            else
            {
                // no sep, just name
                text.clear();
            }
        }

        ui->leInput->setText(QString("/") + name + ' ' + text);
        ui->leInput->setFocus();
    }
}

//

void Chat::updateChat()
{
    ui->tbText->clear();
    QTextCursor cursor = ui->tbText->textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);

    // count items
    int count = 0;
    QListIterator<ChatMessage> it(messages_);
    it.toBack();
    while (it.hasPrevious() && count < maxBlocks_)
    {
        const ChatMessage & msg = it.previous();
        if (filter_.testFlag(msg.type))
        {
            count++;
        }
    }

    // add items
    while (it.hasNext())
    {
        const ChatMessage & msg = it.next();
        if (filter_.testFlag(msg.type))
        {
            cursor.insertHtml(msg.text);
            cursor.insertBlock();
        }
    }
}

void Chat::appendMessage(const ChatMessage & msg)
{
    if (filter_.testFlag(msg.type))
    {
        appendString(msg.text);
        messages_.append(msg);

        // purge
        int maxSize = maxBlocks_ * 5;
        if (messages_.size() > maxSize)
        {
            messages_.erase(messages_.begin(), messages_.begin() + messages_.size() - maxSize);
        }
    }
}

void Chat::appendString(const QString & text)
{
    QTextCursor cursor = ui->tbText->textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    cursor.insertHtml(text);
    cursor.insertBlock();
    ui->tbText->setTextCursor(cursor);
}

void Chat::playerResolved(unsigned id, const QString & pName)
{
    QString name = pName.toHtmlEscaped();
    QString hex = playerIdString(id);

    // update text
    QTextCursor cursor = ui->tbText->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    while (!(cursor = ui->tbText->document()->find(hex, cursor.selectionEnd(), 0)).isNull())
    {
        QTextCharFormat fmt = cursor.charFormat();
        if (fmt.toolTip() == "char_id")
        {
            fmt.setToolTip(QString());
            fmt.setAnchor(true);
            fmt.setAnchorHref(playerAnchorString(id, name));
            fmt.setAnchorName(name);

            cursor.setCharFormat(fmt);
            cursor.insertText(name);
        }
    }

    // update storage
    QString part = playerNoNamedString(id);
    for (ChatMessage & msg : messages_)
    {
        if (msg.text.contains(part))
        {
            QString oldRef = QString("<a href=\"%1\"").arg(playerAnchorString(0, hex));
            QString newRef = QString("<a href=\"%1\"").arg(playerAnchorString(id, name));
            msg.text.replace(part, name).replace(oldRef, newRef);
        }
    }
}

Chat::ChatMessage Chat::formatMessage(ChatType type, unsigned charId, const QString & charName, const QString & message, bool fromMe)
{
    QString text = message.toHtmlEscaped();

    // ?<1><^ff6000[??Удар единорога]> 160кк
    text.replace(QRegularExpression(
        "[\\x{e000}-\\x{e005}]&lt;1&gt;&lt;\\^([0-9A-Fa-f]{6})(.+?)&gt;"),
        "<font color=\"#\\1\">\\2</font>");

    // ?<0><2:44> to :/smile/0/1.gif
    text.replace(QRegularExpression(
        "[\\x{e000}-\\x{e005}]&lt;0&gt;&lt;(\\d):(\\d{1,2})&gt;"),
        "<img src=\":/smile/\\1/\\2.png\" width=\"16\" height=\"16\" />");

    // ?<5><[Infy]><GCBDHJJI> шифт чара
    text.replace(QRegularExpression(
        "[\\x{e000}-\\x{e005}]&lt;\\d&gt;&lt;(\\S+?)&gt;&lt;\\S+&gt;"),
        "\\1");

    QColor baseColor, textColor;
    switch (type)
    {
        case ChatTypeLocal:   baseColor = textColor = TextColorLocal; break;
        case ChatTypeWorld:   baseColor = textColor = TextColorWorld; break;
        case ChatTypeHorn:
        {
            // 49c75701 цвет 49c757 эмоция 01
            baseColor = TextColorWorld;
            if (text.size() >= 8)
            {
                bool ok;
                unsigned int value = text.mid(text.size() - 8, 6).toInt(&ok, 16);
                if (ok)
                {
                    textColor = QColor(value);
                    text.chop(8);
                }
            }
            else
            {
                textColor = baseColor;
            }
            break;
        }
        case ChatTypeClan:    baseColor = textColor = TextColorClan; break;
        case ChatTypeGroup:   baseColor = textColor = TextColorGroup; break;
        case ChatTypePrivate:
            // окончание на !# добавляется к чату, оставленному в кота
            baseColor = textColor = TextColorPrivate;
            if (text.endsWith("!#"))
            {
                text.chop(2);
            }
            break;

        default:
            baseColor = textColor = TextColorLocal; break;
    }

    QString namePart;
    if (charId == 0)
    {
        // system
        // ... и все в куче, да и если подумать, charid 0 не существует
    }
    else
    {
        QString name = charName.toHtmlEscaped();
        if (name.isEmpty())
        {
            if ((charId & CharacterIdFlags::CHARACTER_NPC_FLAG) == 0)   // isPlayer
            {
                QString resolved = ctl_->resolvePlayerName(charId);
                if (!name.isNull())
                {
                    name = resolved.toHtmlEscaped();
                }
            }
            else
            {
                name = playerIdString(charId);
            }
        }

        if (name.isEmpty())
        {
            namePart = QString("<a href=\"%1\" style=\"text-decoration: none\"><font color=#%2>%3</font></a>")
                .arg(playerAnchorString(0, playerIdString(charId)))
                .arg(QString::number(baseColor.lighter(150).rgb(), 16))
                .arg(playerNoNamedString(charId));
        }
        else if (fromMe && type == ChatTypePrivate)
        {
            namePart = tr("You to %1").arg(
                QString("<a href=\"%1\" style=\"text-decoration: none\"><font color=#%2>%3</font></a>")
                .arg(playerAnchorString(charId, name))
                .arg(QString::number(baseColor.lighter(150).rgb(), 16))
                .arg(name)
            );
        }
        else if (fromMe)
        {
            namePart = QString("<font color=#%2>%3</font></a>")
                .arg(QString::number(baseColor.lighter(150).rgb(), 16))
                .arg(name);
        }
        else
        {
            namePart = QString("<a href=\"%1\" style=\"text-decoration: none\"><font color=#%2>%3</font></a>")
                .arg(playerAnchorString(charId, name))
                .arg(QString::number(baseColor.lighter(150).rgb(), 16))
                .arg(name);
        }
    }

    text = (namePart.isEmpty() ? QString("<font color=#%1>%2</font>")
            : QString("%3 : <font color=#%1>%2</font>"))
        .arg(QString::number(textColor.rgb(), 16))
        .arg(text)
        .arg(namePart);

    ChatMessage msg;
    msg.type = type;
    msg.text = text;

    return msg;
}
