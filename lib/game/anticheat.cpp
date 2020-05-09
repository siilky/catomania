
#include "stdafx.h"

#include <chrono>
#include <future>

#include "boost/signal.hpp"

#include "game/anticheat.h"
#include "NetDll.h"
#include "netio/mppc.h"

#define HANDLE_C1(cookies, connection, class, name)  cookies.push_back(connection->bindClientHandler              \
                                                    (std::function<void (const clientdata::Fragment##name *)>   \
                                                    (std::bind(&##class::onC##name, this, _1) )))


AntiCheat::AntiCheat(bool logData /*= false*/)
    : netDll_(0)
    , logData_(logData)
{
    init();
#if defined(_DEBUG)
    logData_ = true;
#endif
}

AntiCheat::~AntiCheat()
{
    unbind();
}

void AntiCheat::init()
{
    futures_.clear();
    lastRemoteCode_.clear();
    delete netDll_;
    netDll_ = new NetDll();
    myId_ = 0;
}

void AntiCheat::bind(std::shared_ptr<Connection> connection)
{
    if (connection_.get() != NULL)
    {
        unbind();
    }

    connection_ = connection;

    HANDLE_S(cookies_, connection_, AntiCheat, ACWhoami);
    HANDLE_S(cookies_, connection_, AntiCheat, ACRemoteCode);
    HANDLE_S(cookies_, connection_, AntiCheat, ACProtoStat);
    HANDLE_S(cookies_, connection_, AntiCheat, ACStatusAnnounce);
    HANDLE_S(cookies_, connection_, AntiCheat, ACReportCheater);
    HANDLE_S(cookies_, connection_, AntiCheat, ACTriggerQuestion);
    HANDLE_S(cookies_, connection_, AntiCheat, ACQuestion);
    HANDLE_S(cookies_, connection_, AntiCheat, ACAnswer);

//     HANDLE_C1(cookies_, connection_, AntiCheat, ACWhoami);
    HANDLE_C1(cookies_, connection_, AntiCheat, ACReport);
//     HANDLE_C1(cookies_, connection_, AntiCheat, ACProtoStat);
//     HANDLE_C1(cookies_, connection_, AntiCheat, ACStatusAnnounce);
//     HANDLE_C1(cookies_, connection_, AntiCheat, ACReportCheater);
//     HANDLE_C1(cookies_, connection_, AntiCheat, ACTriggerQuestion);
//     HANDLE_C1(cookies_, connection_, AntiCheat, ACQuestion);
//     HANDLE_C1(cookies_, connection_, AntiCheat, ACAnswer);
}

void AntiCheat::unbind()
{
    for(const std::future<CodeResult> & f : futures_)
    {
        assert(f.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    }

    if (connection_)
    {
        connection_->unbindHandlers(cookies_);
        connection_.reset();
    }

    init();
}

void AntiCheat::tick(Timestamp /*timestamp*/)
{
    for (auto it = futures_.begin(); it != futures_.end();)
    {
        if (it->wait_for(std::chrono::system_clock::duration::zero()) == std::future_status::ready)
        {
            CodeResult r = it->get();
            if (r.success)
            {
                byte type = 10;
                byte count = 1;
                DWORD result = r.result;

                SerializerLength sl;
                sl.b(type).b(count).w(r.id).l(result);

                barray dataOut;
                dataOut.resize(sl.size());

                SerializerOut s(dataOut);
                s.b(type).b(count).w(r.id).l(result);

                // std::shared_ptr<MPPCEncoder> enc(new MPPCEncoder);
                // barray packed = enc->transform(dataOut);

                clientdata::FragmentACReport answer;
                answer.myID = myId_;
                answer.unpackedSize = dataOut.size();
                answer.data = dataOut;
                connection_->send(&answer);
            }
            else
            {
                // logged by dll
            }
            
            it = futures_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

//

void AntiCheat::onACWhoami(const serverdata::FragmentACWhoami *f)
{
    logData("S Whoami", f->bytes());
}

void AntiCheat::onACRemoteCode(const serverdata::FragmentACRemoteCode *f)
{
//     if (lastRemoteCode_.empty()
//         || lastRemoteCode_ != f->bytes())
    {
//         lastRemoteCode_ = f->bytes();
        logData("S Code", f->bytes());
    }
    
    if (myId_ == 0)
    {
        myId_ = f->id;
    }
    assert(myId_ == f->id);

    for (size_t i = 0; i < f->blocks.size(); ++i)
    {
        switch (f->blocks[i].type)
        {
            case serverdata::FragmentACRemoteCode::DataBlock:
            {
                netDll_->addString(f->blocks[i].id, f->blocks[i].data);
                break;
            }
            case serverdata::FragmentACRemoteCode::ImportFunction:
            {
                netDll_->addFunctionRef(f->blocks[i].id, f->blocks[i].data);
                break;
            }
            case serverdata::FragmentACRemoteCode::ExecuteCode:
            {
                assert(f->blocks[i].data.empty());
                futures_.push_back(std::async(std::launch::async, callNetDll, netDll_, f->blocks[i].id));
                break;
            }
            default:
            {
                barray imports;
                int refSize = 0;
                if (f->blocks[i].type > 6)
                {
                    refSize = f->blocks[i].type - 6;
                    std::copy(f->blocks[i].data.begin(), f->blocks[i].data.begin() + refSize, std::back_inserter(imports));
                }
                netDll_->addCode(f->blocks[i].id, barray(f->blocks[i].data.begin() + refSize, f->blocks[i].data.end()), imports);
                break;
            }
        }
    }
}

void AntiCheat::onACProtoStat(const serverdata::FragmentACProtoStat *f)
{
    logData("S Stat", f->bytes());
}

void AntiCheat::onACStatusAnnounce(const serverdata::FragmentACStatusAnnounce *f)
{
    logData("S Status Announce", f->bytes());
}

void AntiCheat::onACReportCheater(const serverdata::FragmentACReportCheater *f)
{
    logData("S Report", f->bytes());
}

void AntiCheat::onACTriggerQuestion(const serverdata::FragmentACTriggerQuestion *f)
{
    logData("S Trigger Question", f->bytes());
}

void AntiCheat::onACQuestion(const serverdata::FragmentACQuestion *f)
{
    logData("S Question", f->bytes());
}

void AntiCheat::onACAnswer(const serverdata::FragmentACAnswer *f)
{
    logData("S Answer", f->bytes());
}

void AntiCheat::onCACReport(const clientdata::FragmentACReport * f)
{
    logData("C Report", f->bytes());
}

// void AntiCheat::onCACWhoami(const clientdata::FragmentACWhoami *f)
// {
// }
// 
// void AntiCheat::onCACProtoStat(const clientdata::FragmentACProtoStat *f)
// {
// }
// 
// void AntiCheat::onCACStatusAnnounce(const clientdata::FragmentACStatusAnnounce *f)
// {
// }
// 
// void AntiCheat::onCACReportCheater(const clientdata::FragmentACReportCheater *f)
// {
// }
// 
// void AntiCheat::onCACTriggerQuestion(const clientdata::FragmentACTriggerQuestion *f)
// {
// }
// 
// void AntiCheat::onCACQuestion(const clientdata::FragmentACQuestion *f)
// {
// }
// 
// void AntiCheat::onCACAnswer(const clientdata::FragmentACAnswer *f)
// {
// }

//

void AntiCheat::logData(const char * name, const barray & data) const
{
    static const char *filename = "acd.data";
    if ( ! logData_ || ! std::ifstream(filename).good())
    {
        return;
    }

    tofstream file(filename, std::ios_base::out | std::ios_base::app);

    file << '[' << hex << setw(8) << time(0) << "]  " << name << endl;
    dumpBytes(file, data.begin(), data.end());
}

AntiCheat::CodeResult AntiCheat::callNetDll(NetDll *netDll_, WORD id)
{
    CodeResult r;
    r.id = id;
    r.success = netDll_->executeCodeBlock(id, r.result);
    return r;
}
