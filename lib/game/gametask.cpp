
#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/gametask.h"
#include "netdata/fragments_npc.h"
#include "netdata/fragments_client.h"
#include "netdata/fragments_giclient.h"
#include "game/ai/task.h"

GameTask::GameTask()
{
    init();

    taskRoot_ =
        task<TLoop>
        (
            (
                task<TDelay>(500)
                & task<TEvent>(std::bind(&GameTask::onTimer, this))
            )
        );
}


GameTask::~GameTask()
{
    unbind();
}

void GameTask::init()
{
    tasks_.clear();
    tasksAutoaccept_.clear();
    lastTs_ = 0;
}

void GameTask::bind(std::shared_ptr<Connection> connection)
{
    connection_ = connection;

    HANDLE_S_GI(cookies_, connection_, GameTask, TaskVarData);

    init();
}

void GameTask::unbind()
{
    if (connection_)
    {
        connection_->unbindHandlers(cookies_);
        connection_.reset();
    }
}

void GameTask::tick(Timestamp timestamp)
{
    lastTs_ = timestamp;
    taskRoot_->tick(timestamp);
}

void GameTask::takeTask(DWORD taskId, bool autoAccept /*= true*/)
{
    if (!connection_)
    {
        return;
    }

    if (autoAccept
        && std::find(tasksAutoaccept_.begin(), tasksAutoaccept_.end(), taskId) == tasksAutoaccept_.end())
    {
        Log("Added autoaccept of %u", taskId);
        tasksAutoaccept_.push_back(taskId);
    }

    clientdata::FragmentRequestTask *f = new clientdata::FragmentRequestTask;
    f->taskId = taskId;
    f->unk1 = 0;
    f->unk2 = 0;
    clientdata::FragmentGiNpcInteract *fn = new clientdata::FragmentGiNpcInteract(f);
    clientdata::FragmentGameinfoSet fg(fn);
    connection_->send(&fg);
}

void GameTask::acceptTask(DWORD taskId)
{
    if (!connection_)
    {
        return;
    }

    Log("Accepting task %u", taskId);

    std::vector<Task>::iterator it = std::find(tasks_.begin(), tasks_.end(), Task(taskId));
    if (it == tasks_.end())
    {
        Log("Task not exists %u, skipping", taskId);
        return;
    }

    if (it->accepted == 0)
    {
        it->accepted = lastTs_;
        sendAccept(taskId);
    }
    else
    {
        Log("Already accepting");
    }
}

//

void GameTask::onTaskVarData(const serverdata::FragmentGiTaskVarData *f)
{
    Log("TaskVarData %i", f->command);

    switch (f->command)
    {
        case serverdata::TaskVarDataImpl::CmdAddTask:
        {
            if (f->data.size() < 6)
            {
                Log("Add task data too short");
                break;
            }

            barray::const_iterator pos = f->data.begin();
            WORD taskId = getWORD_r(pos);
            DWORD timestamp = getDWORD_r(pos);

            Task newTask(taskId, timestamp);
            if (std::find(tasks_.begin(), tasks_.end(), newTask) == tasks_.end())
            {
                tasks_.push_back(newTask);
                Log("Added task %u", newTask.id);

                // autoaccept
                std::vector<DWORD>::iterator it = std::find(tasksAutoaccept_.begin(), tasksAutoaccept_.end(), taskId);
                if ( it != tasksAutoaccept_.end())
                {
                    acceptTask(taskId);
                    tasksAutoaccept_.erase(it);
                }
            }
    	    break;
        }
        case serverdata::TaskVarDataImpl::CmdRemoveTask:
        {
            if (f->data.size() < 2)
            {
                Log("Remove task data too short");
                break;
            }

            barray::const_iterator pos = f->data.begin();
            WORD taskId = getWORD_r(pos);

            {
                Task newTask(taskId /*, f->timestamp*/);
                std::vector<Task>::iterator it = std::find(tasks_.begin(), tasks_.end(), newTask);
                if (it != tasks_.end())
                {
                    tasks_.erase(it);
                    Log("Removed task %u", newTask.id);
                }
            }

            // remove autoaccept
            {
                std::vector<DWORD>::iterator it = std::find(tasksAutoaccept_.begin(), tasksAutoaccept_.end(), taskId);
                if (it != tasksAutoaccept_.end())
                {
                    tasksAutoaccept_.erase(it);
                }
            }

            break;
        }
    }
}

void GameTask::onTimer()
{
    for (std::vector<Task>::iterator it = tasks_.begin(); it != tasks_.end(); ++it)
    {
        if (it->accepted != 0
            && (lastTs_ - it->accepted) > AcceptTimeout)
        {
            Log("Retrying accepting task %u", it->id);
            sendAccept(it->id);
        }
    }
}

void GameTask::sendAccept(DWORD taskId)
{
    clientdata::FragmentGiActivateTask *f = new clientdata::FragmentGiActivateTask();
    f->option = 0x03;
    f->command = clientdata::FragmentGiActivateTask::CmdActivate;
    f->taskId = (WORD) taskId;

    clientdata::FragmentGameinfoSet fg(f);
    connection_->send(&fg);
}
