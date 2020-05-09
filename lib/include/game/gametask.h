#ifndef gametask_h
#define gametask_h

#include "netdata\connection.h"
#include "netdata\fragments_giserver.h"

class ITask;

class GameTask
{
public:
    struct Task
    {
        DWORD       id;
        Timestamp   added;
        Timestamp   accepted;   // last accepted time

        Task(DWORD id_, Timestamp added_ = 0)
            : id(id_)
            , added(added_)
            , accepted(0)
        {}

        bool operator==(const Task & r) const
        {
            return id == r.id;
        }
    };

    GameTask();
    ~GameTask();

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    void tick(Timestamp timestamp);

    //

    void takeTask(DWORD taskId, bool autoAccept = true);
    void acceptTask(DWORD taskId);

    void sendAccept(DWORD taskId);

private:
    enum
    {
        AcceptTimeout   = 5000,     // ms
    };

    void onTimer();
    void onTaskVarData(const serverdata::FragmentGiTaskVarData *f);

    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> cookies_;

    std::shared_ptr<ITask>        taskRoot_;      // engine tasks
    Timestamp                       lastTs_;

    std::vector<Task>   tasks_;
    std::vector<DWORD>  tasksAutoaccept_;           // queue we need to accept before task is actually received
};

#endif