#ifndef tnpc_h_
#define tnpc_h_


class TSelectTarget : public ITask
{
public:
    TSelectTarget(std::shared_ptr<Game> game);

    DWORD   targetId;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onEvent(BaseEvent *event);

    std::shared_ptr<Game>     game_;
};


class TOpenNpc : public ITask
{
public:
    TOpenNpc(std::shared_ptr<Game> game);

    DWORD   npcId;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onNpcGreeting(const serverdata::NpcGreetingImpl * /*f*/);

    std::shared_ptr<Game>     game_;
    Connection::Cookie          cNpcGreeting_;
};

#endif
