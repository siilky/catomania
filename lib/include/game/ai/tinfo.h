#ifndef tinfo_h
#define tinfo_h


class TGetFriendlist : public ITask
{
    // Get friend list

public:
    TGetFriendlist(std::shared_ptr<Game> game)
        : game_(game)
    {}

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onFriendListRe(const serverdata::FragmentGetFriendsRe * /*f*/);

    std::shared_ptr<Game> game_;
    Connection::Cookie      cFriendList_;
};


#endif
