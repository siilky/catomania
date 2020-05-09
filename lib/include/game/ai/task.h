// $Id: task.h 1777 2020-03-17 13:45:59Z jerry $
//
#ifndef task_h
#define task_h

#include "game/ai/condition.h"
#include "game/gtime.h"
#include "game/game.h"

// Task lifetime    state
//  initial         NONE
//
//   ->tick()
//   state [NONE] -> checkstartCondition() - 0 -> .
//                   ----------------------- 1 --- onStarted() \
//         [RUNNING] ------------------------------------------> RUNNING -> tick() -> !RUNNING -> .
//                                                                                 -> RUNNING  -> checkStopCondition() [true]  -> COMPLETED -> onCompleted()
//                                                                                                              [false] -> checkFailCondition() [true]  -> FAILED -> onFailed()
//                                                                                                                                            [false] -> RUNNING -> .
// Task Combinations
//                    run            completion         failure         syntax
//
//     Sequence     one by one      all completed	    first fail      (A & B)
//     ?            one by one      all completed       all failed
//     -            one by one      first completed	    first fail
//     Selector     one by one      first completed     all failed      (A | B)
//     Parallel     all together    all completed       first fail      (A && B)
//     -            all together    all completed       all failed
//     Alternate    all together    first completed     first fail      (A ^ B)
//     Joint        all together    first completed	    all failed      (A || B)
//
// Expressions are evaluated in left-to-right order, for example
// A & B | C is equivalent to ((A & B) | C)

class ITask
{
public:
    typedef enum
    {
        TASK_NONE,
        TASK_RUNNING,
        TASK_COMPLETED,
        TASK_FAILED,
        TASK_TIMED_OUT,
    } State;

    ITask()
        : state_(TASK_NONE)
        , completedStart_(false)
    {}

    virtual ~ITask()
    {
        stop();
    }

    bool isRunning() const
    {
        return state_ == TASK_RUNNING;
    }

    virtual void stop()
    {
        // if state is completed/failed that means it was set by any "async" routine
        // but tick() and onStopped() weren't called yet

        if (state_ != TASK_NONE)
        {
            onStopped();
        }
        state_ = TASK_NONE;
    }

    void startWhen(const std::shared_ptr<Condition> & cond)
    {
        // waits for condition before execution
        startCondition_ = cond;
    }

    void startIf(const std::shared_ptr<Condition> & cond)
    {
        // completes when condition is false and does not wait for condition before execution
        startCondition_ = cond;
        completedStart_ = true;
    }

    void stopWhen(const std::shared_ptr<Condition> & cond)
    {
        stopCondition_ = cond;
    }

    void failWhen(const std::shared_ptr<Condition> & cond)
    {
        failCondition_ = cond;
    }

    State tick(Timestamp timestamp)
    {
        if (state_ == TASK_NONE)
        {
            if (startCondition_)
            {
                if ( ! (*startCondition_)())
                {
                    if (completedStart_)
                    {
                        state_ = TASK_COMPLETED;
                        return state_;
                    }
                    else
                    {
                        // return RUNNING but internal state is NONE
                        return TASK_RUNNING;
                    }
                }
                else
                {
                    Log("Task started by run condition");
                }
            }

            state_ = onStarted();            // onStarted can override running state
        }

        // state could switch to completed/failed/etc outside of tick() function, for example in packet handler
        // so we don't need state failure check here

        if (state_ == TASK_RUNNING)
        {
            state_ = tick_(timestamp);

            // optimistic scenario: if both failed and completed are triggered
            // then 'completed' takes more priority

            if (failCondition_ && (*failCondition_)())
            {
                Log("Task failed by fail condition");
                state_ = TASK_FAILED;
            }

            if (stopCondition_ && (*stopCondition_)())
            {
                Log("Task stopped by stop condition");
                state_ = TASK_COMPLETED;
            }
        }

        // tick handler can change resulting status
        callEvents();

        if (state_ == TASK_RUNNING)
        {
            return state_;
        }

        onStopped();

        // set internal state to NONE to allow next start
        State toReturn = state_;
        state_ = TASK_NONE;
        return toReturn;
    }

protected:

//     friend class IComplexTask;
//     friend class TaskSequence;
//     friend class TaskSelector;
//     friend class TaskParallel;
//     friend class TaskJoint;
//     friend class TaskAlternate;

    virtual State tick_(Timestamp /*timestamp*/)
    {
        return state_;
    }

    // Events

    // Called when task is about to start to set the needed environment up.
    // Should return RUNNING if task is ok to run or any other state except NONE.
    virtual State onStarted()       { return TASK_RUNNING; }

    // called when task executes its final time slice. Task should release resources taken
    // and stop any action it's currently doing if it's possible.
    virtual void onStopped()        {}

    // Completion events

    // Called when task end with completed state
    virtual State onCompleted()    { return state_; }

    // Called when task end with failed state
    virtual State onFailed()       { return state_; }

    // Called when task end with timed out state
    virtual State onTimedOut()     { return state_; }

    //

    void callEvents()
    {
        switch (state_)
        {
            case TASK_COMPLETED:    state_ = onCompleted(); break;
            case TASK_FAILED:       state_ = onFailed();    break;
            case TASK_TIMED_OUT:    state_ = onTimedOut();  break;
            case TASK_RUNNING:      /* */                   break;
            default:     assert(0); state_ = TASK_FAILED;   break;
        }
    }

    // data

    State                           state_;
    std::shared_ptr<Condition>    startCondition_, stopCondition_, failCondition_;
    bool                            completedStart_;
};

//
// Class to implement compound tasks so they build complex task in constructor like
//
// task_ = (*task<Task1>() & *task<Task2>().stopWhen(GCondition<Cond>) ) | task<Task3>() etc
//
class IComplexTask : public ITask
{
public:

    virtual void stop()
    {
        task_->stop();
        ITask::stop();  // super.
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        return task_->tick(timestamp);
    }

    std::shared_ptr<ITask>    task_;
};

// Task combinations

class TaskBinaryOp : public ITask
{
public:
    TaskBinaryOp(const std::shared_ptr<ITask> & l, const std::shared_ptr<ITask> & r)
        : l_(l), r_(r)
    {}

    virtual void onStopped()
    {
        l_->stop();
        r_->stop();
    }

protected:
    std::shared_ptr<ITask>    l_, r_;
};


class TaskSequence : public TaskBinaryOp
{
//                    run            completion         failure         syntax
//     Sequence     one by one      all completed	    first fail      (A & B)

public:
    TaskSequence(const std::shared_ptr<ITask> & l, const std::shared_ptr<ITask> & r)
        : TaskBinaryOp(l, r)
    {}

    virtual State onStarted()
    {
        left_ = true;
        return TASK_RUNNING;
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        State result = TASK_FAILED;

        if (left_)
        {
            result = l_->tick(timestamp);

            if (result == TASK_COMPLETED)
            {
                left_ = false;
            }
        }

        if ( ! left_)
        {
            result = r_->tick(timestamp);
        }

        return result;
    }

private:
    bool    left_;
};

class TaskSelector : public TaskBinaryOp
{
//                    run            completion         failure         syntax
//     Selector     one by one      first completed     all failed      (A | B)

public:
    TaskSelector(const std::shared_ptr<ITask> & l, const std::shared_ptr<ITask> & r)
        : TaskBinaryOp(l, r)
    {}

    virtual State onStarted()
    {
        left_ = true;
        return TASK_RUNNING;
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        State result = TASK_FAILED;

        if (left_)
        {
            result = l_->tick(timestamp);

            if (result == TASK_RUNNING || result == TASK_COMPLETED)
            {
                return result;
            }

            left_ = false;
        }

        if ( ! left_)
        {
            result = r_->tick(timestamp);
        }

        return result;
    }

private:
    bool    left_;
};

class TaskParallel : public TaskBinaryOp
{
//                    run            completion         failure         syntax
//     Parallel     all together    all completed       first fail      (A && B)

public:
    TaskParallel(const std::shared_ptr<ITask> & l, const std::shared_ptr<ITask> & r)
        : TaskBinaryOp(l, r)
    {}

    virtual State onStarted()
    {
        // set results initially to running to let tasks to tick first
        resultL = resultR = TASK_RUNNING;
        return TASK_RUNNING;
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        if (resultL == TASK_RUNNING)
        {
            resultL = l_->tick(timestamp);
        }

        if (resultR == TASK_RUNNING)
        {
            resultR = r_->tick(timestamp);
        }

        if (resultL == TASK_COMPLETED && resultR == TASK_COMPLETED)
        {
            return TASK_COMPLETED;
        }

        if (resultL != TASK_RUNNING && resultL != TASK_COMPLETED)
        {
            return resultL;
        }
        else if (resultR != TASK_RUNNING && resultR != TASK_COMPLETED)
        {
            return resultR;
        }

        return TASK_RUNNING;
    }

private:
    State   resultL, resultR;

};

class TaskJoint : public TaskBinaryOp
{
//                    run            completion         failure         syntax
//     Joint        all together    first completed	    all failed      (A || B)

public:
    TaskJoint(const std::shared_ptr<ITask> l, const std::shared_ptr<ITask> & r)
        : TaskBinaryOp(l, r)
        , resultL(TASK_NONE), resultR(TASK_NONE)
    {}

    virtual State onStarted()
    {
        // set results initially to running to let tasks to tick first
        resultL = resultR = TASK_RUNNING;
        return TASK_RUNNING;
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        if (resultL == TASK_RUNNING)
        {
            resultL = l_->tick(timestamp);
        }

        if (resultR == TASK_RUNNING)
        {
            resultR = r_->tick(timestamp);
        }

        if (resultL == TASK_COMPLETED || resultR == TASK_COMPLETED)
        {
            return TASK_COMPLETED;
        }

        // any COMPLETED gets out above so we check here for failures
        if (resultL != TASK_RUNNING && resultR != TASK_RUNNING)
        {
            return resultL;
        }

        return TASK_RUNNING;
    }

private:
    State   resultL, resultR;
};

class TaskAlternate : public TaskBinaryOp
{
    //                    run            completion         failure         syntax
    //     Alternate    all together    first completed     first fail      (A ^ B)

public:
    TaskAlternate(const std::shared_ptr<ITask> & l, const std::shared_ptr<ITask> & r)
        : TaskBinaryOp(l, r)
    {}

    virtual State onStarted()
    {
        // set results initially to running to let tasks to tick first
        resultL = resultR = TASK_RUNNING;
        return TASK_RUNNING;
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        if (resultL == TASK_RUNNING)
        {
            resultL = l_->tick(timestamp);
        }

        if (resultR == TASK_RUNNING)
        {
            resultR = r_->tick(timestamp);
        }

        if (resultL == TASK_COMPLETED || resultR == TASK_COMPLETED)
        {
            return TASK_COMPLETED;
        }

        if (resultL != TASK_RUNNING && resultL != TASK_COMPLETED)
        {
            return resultL;
        }
        else if (resultR != TASK_RUNNING && resultR != TASK_COMPLETED)
        {
            return resultR;
        }

        return TASK_RUNNING;
    }

private:
    State   resultL, resultR;
};

// *** Utils ***

class TWait : public ITask
{
//                run        completion     failure     timeout
//     Wait      always       	-              -        timeout

public:
    TWait(Timestamp timeout = 0)
        : timeout_(timeout)
        , endTime_(0)
    {}

    virtual State onStarted()
    {
        endTime_ = 0;
        return TASK_RUNNING;
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        if (timeout_ != 0)
        {
            if (endTime_ == 0)
            {
                endTime_ = timestamp + timeout_;
                // Log("Wait of %llu will end in %llu (cur %llu)", timeout_, endTime_, timestamp);
            }

            if (timestamp > endTime_)
            {
                // Log("Wait of %llu timed out at %llu (cur %llu)", timeout_, endTime_, timestamp);
                return TASK_TIMED_OUT;
            }
        }
        return TASK_RUNNING;
    }

private:
    Timestamp  endTime_;       // ms
    Timestamp  timeout_;       // ms
};

class TDelay : public TWait
{
//                run        completion     failure     timeout
//     Wait      always      timeout          -           -

public:
    TDelay(Timestamp delay)    // msec
        : TWait(delay)
    {}

private:
    virtual State onTimedOut()
    {
        // Log("Delay done");
        return TASK_COMPLETED;
    }
};

class TRandomDelay : public ITask
{
//                run        completion     failure     timeout
//     Wait      always      timeout          -           -

public:
    TRandomDelay()
        : fixedPart(0), randomPart(0), endTime_(0)
    {}

    void reset()       // completes timeout
    {
        state_ = TASK_COMPLETED;
    }

    Timestamp  fixedPart;       // ms
    Timestamp  randomPart;      // ms

protected:
    virtual State tick_(Timestamp timestamp)
    {
        if (endTime_ == 0)
        {
            endTime_ = timestamp + fixedPart + Timestamp((double)rand() / RAND_MAX * randomPart);
            // Log("Wait of %llu will end in %llu (cur %llu)", timeout_, endTime_, timestamp);
        }

        if (timestamp > endTime_)
        {
            // Log("Wait of %llu timed out at %llu (cur %llu)", timeout_, endTime_, timestamp);
            return TASK_COMPLETED;
        }

        return TASK_RUNNING;
    }

    virtual State onStarted()
    {
        endTime_ = 0;
        return TASK_RUNNING;
    }

private:
    Timestamp  endTime_;       // ms
};

class TTimer : public ITask
{
    // Timer with guaranteed ticks count during run time
    //                run        completion     failure     timeout
    //               always       !callback        -           -

public:
    TTimer(const std::shared_ptr<Game> & game, Timestamp period, std::function<bool()> cb)
        : game_(game)
        , period_(period)
        , cb_(cb)
        , lastFired_(0)
    {
        assert(cb_);
    }

    virtual State onStarted()
    {
        Log("Starting timer with period of %llu", period_);
        lastFired_ = game_->timer()->time();
        return TASK_RUNNING;
    }

protected:
    virtual State tick_(Timestamp timestamp)
    {
        bool continu = true;
        if (timestamp > (lastFired_ + period_))
        {
            lastFired_ = lastFired_ + period_;

            continu = cb_();
        }
        return continu ? TASK_RUNNING : TASK_COMPLETED;
    }

private:
    std::shared_ptr<Game> game_;
    Timestamp               period_, lastFired_;
    std::function<bool()> cb_;
};


class TEvent : public ITask
{
    // Callback event task
    //  run        completion     failure     timeout
    //  once          -              -           -

public:
    TEvent(std::function<void ()> event)
        : event_(event)
    {
        assert(event_);
    }

    virtual State onStarted()
    {
        // Log("Event");
        event_();
        return TASK_COMPLETED;
    }

private:
    std::function<void ()> event_;
};


class TOnce : public ITask
{
    // Ticked once
    //  run        completion     failure     timeout
    //  once          1              -           -

protected:
    virtual State tick_(Timestamp)
    {
        return TASK_COMPLETED;
    }
};


//

class TaskPtrProxy
{
public:
    TaskPtrProxy(const std::shared_ptr<ITask> & p)
        : task_(p)
    {}

    operator const std::shared_ptr<ITask>&() const
    {
        return task_;
    }

    TaskPtrProxy & startWhen(const std::shared_ptr<Condition> & cond)
    {
        task_->startWhen(cond);
        return *this;
    }

    TaskPtrProxy & startIf(const std::shared_ptr<Condition> & cond)
    {
        task_->startIf(cond);
        return *this;
    }

    TaskPtrProxy & stopWhen(const std::shared_ptr<Condition> & cond)
    {
        task_->stopWhen(cond);
        return *this;
    }

    TaskPtrProxy & failWhen(const std::shared_ptr<Condition> & cond)
    {
        task_->failWhen(cond);
        return *this;
    }

private:
    std::shared_ptr<ITask>    task_;
};

// Functions

inline TaskPtrProxy operator &(TaskPtrProxy l, TaskPtrProxy r)
{
    return std::shared_ptr<ITask>(new TaskSequence(l, r));
}

inline TaskPtrProxy operator |(TaskPtrProxy l, TaskPtrProxy r)
{
    return std::shared_ptr<ITask>(new TaskSelector(l, r));
}

inline TaskPtrProxy operator &&(TaskPtrProxy l, TaskPtrProxy r)
{
    return std::shared_ptr<ITask>(new TaskParallel(l, r));
}

inline TaskPtrProxy operator ^(TaskPtrProxy l, TaskPtrProxy r)
{
    return std::shared_ptr<ITask>(new TaskAlternate(l, r));
}

inline TaskPtrProxy operator ||(TaskPtrProxy l, TaskPtrProxy r)
{
    return std::shared_ptr<ITask>(new TaskJoint(l, r));
}

template<typename Task>
TaskPtrProxy task()
{
    return std::shared_ptr<ITask>(new Task());
}

template<typename Task, typename Arg1>
TaskPtrProxy task(Arg1 a)
{
    return std::shared_ptr<ITask>(new Task(a));
}

template<typename Task, typename Arg1, typename Arg2>
TaskPtrProxy task(Arg1 a, Arg2 b)
{
    return std::shared_ptr<ITask>(new Task(a, b));
}

template<typename Task, typename Arg1, typename Arg2, typename Arg3>
TaskPtrProxy task(Arg1 a, Arg2 b, Arg3 c)
{
    return std::shared_ptr<ITask>(new Task(a, b, c));
}

template<typename Arg1>
TaskPtrProxy taskFail(Arg1 a)
{
    return std::shared_ptr<ITask>(new TFail(a));
}

// *** Decorators ***

class TFail : public IComplexTask
{
    //               completion     failure     timeout
    //     Fail      failed         failure     timeout
public:
    TFail(const std::shared_ptr<ITask> & task)
    {
        task_ = task;
    }

protected:
    // Completion events

    virtual State onCompleted()
    {
        return TASK_FAILED;
    }
};

class TSucceed : public IComplexTask
{
    //               completion     failure     timeout
    //   Succeed        any            -           -
public:
    TSucceed(const std::shared_ptr<ITask> & task)
    {
        task_ = task;
    }

protected:
    // Completion events
    virtual void onStopped()
    {
        state_ = TASK_COMPLETED;
    }
};


class TLoop : public IComplexTask
{
    //                    run                     completion             failure    timeout
    //     Loop     while current < iterations   iterations reached	    first fail     -

public:
    TLoop(const std::shared_ptr<ITask> & task, unsigned iterations = 0)
        : iterations_(iterations), current_(0)
    {
        task_ = task;
    }

    virtual State onStarted()
    {
        Log("Starting loop with %i", iterations_);

        current_ = 0;
        return TASK_RUNNING;
    }

protected:
    virtual State onCompleted()
    {
        if (iterations_ != 0)
        {
            Log("Next iteration of loop with %i (%i)", iterations_, current_);

            current_ ++;

            if (current_ >= iterations_)
            {
                return TASK_COMPLETED;
            }
        }
        return TASK_RUNNING;
    }

private:
    unsigned    iterations_, current_;
};

class TCLoop : public IComplexTask
{
    //                    run           completion       failure    timeout
    //     FLoop     while condition   !condition	    first fail     -

public:
    TCLoop(const std::shared_ptr<Condition> & condition, const std::shared_ptr<ITask> & task)
        : condition_(condition)
    {
        task_ = task;
    }

    virtual State onStarted()
    {
        if ((*condition_)())
        {
            Log("Starting conditional loop");
            return TASK_RUNNING;
        }
        else
        {
            Log("Conditional loop completed by condition");
            return TASK_COMPLETED;
        }
    }

protected:
    virtual State onCompleted()
    {
        if ((*condition_)())
        {
            return TASK_RUNNING;
        }
        else
        {
            return TASK_COMPLETED;
        }
    }

private:
    std::shared_ptr<Condition> condition_;
};

class TTimeout : public IComplexTask
{
public:
    TTimeout(const std::shared_ptr<ITask> & t, Timestamp timeout)
    {
        task_ = t ^ task<TWait>(timeout);
    }
};



#endif