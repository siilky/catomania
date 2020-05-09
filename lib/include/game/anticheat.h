#ifndef anticheat_h_
#define anticheat_h_

#include <list>
#include <future>

#include "netdata\fragments_server.h"
#include "netdata\fragments_client.h"
#include "netdata\connection.h"

class NetDll;

class AntiCheat
{
public:
    AntiCheat(bool logData = false);
    ~AntiCheat();

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    void tick(Timestamp timestamp);

private:
    struct CodeResult
    {
        WORD        id;
        bool        success;
        unsigned    result;
    };

    void onACWhoami(const serverdata::FragmentACWhoami *f);
    void onACRemoteCode(const serverdata::FragmentACRemoteCode *f);
    void onACProtoStat(const serverdata::FragmentACProtoStat *f);
    void onACStatusAnnounce(const serverdata::FragmentACStatusAnnounce *f);
    void onACReportCheater(const serverdata::FragmentACReportCheater *f);
    void onACTriggerQuestion(const serverdata::FragmentACTriggerQuestion *f);
    void onACQuestion(const serverdata::FragmentACQuestion *f);
    void onACAnswer(const serverdata::FragmentACAnswer *f);

    void onCACReport(const clientdata::FragmentACReport *f);
//     void onCACWhoami(const clientdata::FragmentACWhoami *f);
//     void onCACProtoStat(const clientdata::FragmentACProtoStat *f);
//     void onCACStatusAnnounce(const clientdata::FragmentACStatusAnnounce *f);
//     void onCACReportCheater(const clientdata::FragmentACReportCheater *f);
//     void onCACTriggerQuestion(const clientdata::FragmentACTriggerQuestion *f);
//     void onCACQuestion(const clientdata::FragmentACQuestion *f);
//     void onCACAnswer(const clientdata::FragmentACAnswer *f);

    void logData(const char *name, const barray & data) const;

    static CodeResult callNetDll(NetDll  *netDll_, WORD id);

    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> cookies_;

    barray  lastRemoteCode_;
    bool    logData_;

    NetDll  *netDll_;
    DWORD   myId_;

    std::list<std::future<CodeResult>>  futures_;

};


#endif