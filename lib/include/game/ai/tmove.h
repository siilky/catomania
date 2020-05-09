#ifndef tmove_h
#define tmove_h


class TMove : public ITask
{
public:
    TMove(std::shared_ptr<Game> game);

    Coord3D     destination;
    //bool        useFlightAccelerate; TBD

    enum {  MoveTickInterval = 500 };
    enum {  TracePosMax = 10 };

protected:
    virtual State tick_(Timestamp timestamp);
    virtual State onStarted();
    virtual void onStopped();

private:
    State sendMove(Timestamp timestamp, bool forceStop = false);
    void onSelfTraceCurPos(const serverdata::FragmentGiSelfTraceCurPos * f);

    std::shared_ptr<Game> game_;
    Timestamp               lastMoveStamp_;
    bool                    isMoving_;

    Connection::Cookie      cTracePos_;
    int                     traceCtr_;
};

#endif