#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include "qlib\game\gamebase.h"

namespace Ui {class Chat;}
class CatCtl;

class Chat : public QWidget {
    Q_OBJECT
public:
    Chat(QSharedPointer<CatCtl> ctl, QWidget * parent = Q_NULLPTR);
    ~Chat();

    void setBufferSize(int chatBufferOption);

    void setEnabled(bool isEnabled = true);

private slots:
    void on_leInput_textChanged(const QString & text);
    void on_leInput_returnPressed();

    void disableInput();
    void enableInput();

    void updateFilters();
    void onEvent(const qlib::GameEvent & event);


    void onAnchorClicked(const QUrl & link);

private:
    enum ChatType
    {
        // bitflags for fillet
        ChatTypeLocal   = 0x01,
        ChatTypeWorld   = 0x02,
        ChatTypeHorn    = 0x04,
        ChatTypeClan    = 0x08,
        ChatTypeGroup   = 0x10,
        ChatTypePrivate = 0x20,
    };

    struct ChatMessage
    {
        ChatType    type;
        QString     text;
    };

    void updateChat();
    void appendMessage(const ChatMessage & msg);
    void appendString(const QString & msg);
    void playerResolved(unsigned playerId, const QString & name);
    ChatMessage formatMessage(ChatType type, unsigned charId, const QString & charName, const QString & message, bool fromMe = false);

    Ui::Chat *ui;
    QSharedPointer<CatCtl> ctl_;

    QFlags<ChatType>    filter_;
    QList<ChatMessage>  messages_;

    int     maxBlocks_;
};

#endif