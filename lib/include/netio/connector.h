#ifndef connector_h_
#define connector_h_

#include <iostream>
#include <fstream>

#include "encoder.h"
#include "PWUnpack.h"
#include "mppc.h"


class Connector

{
protected:
    Connector();
    virtual ~Connector() {};

    bool setAuth( const std::string   & login
                , const std::string   & password
                , bool                  forced = false);

    bool setPreEncoding(int clientOption, const barray & clientKey,
                        int serverOption, const barray & serverKey);

    barray processDataIn(const barray & data);
    barray processDataOut(const barray & data);
    void processTimer();

    virtual void sendReply(const barray & /*data*/) { assert(0); };     // called from processDataIn to send raw data
    virtual void fail(const std::wstring & /*msg*/) { assert(0); };


    void logBytes(const char *prefix, const barray & data);

    bool authenticated() const
    {
        return authenticated_;
    }

private:
    enum
    {
        MAX_SERVR_KEEPALIVE     = 5,
    };

    #ifdef _DEBUG
        std::ofstream   logfile_;
    #endif

    std::string login_;
    std::string password_;
    bool        forcedLogin_ = false;
    bool        authenticated_ = false;

    // comeback data
    std::vector<std::string>    comebackData_;

    barray      aKey;               // auth key
    RC4Encoder  clientEnc;          // шифрование исходящих пакетов
    RC4Encoder  serverEnc;          // шифрование входящих пакетов
    CUnpack     unpacker_;
    //MPPCDecoder mppcDecoder_;

    bool        doPreEndoding_ = false;
    RC4Encoder  clientPreEnc_;
    RC4Encoder  serverPreEnc_;

    int         keepaliveMisses_ = 0;
};


class ConnectorServerEp
{

protected:
    ConnectorServerEp();

    barray processDataIn(const barray & data);
    barray processDataOut(const barray & data);

    virtual void sendReply(const barray & /*data*/) { assert(0); };     // called from processDataIn to send raw data
    virtual void fail(const std::wstring & /*msg*/) { assert(0); };

    bool authenticated() const
    {
        return authenticated_;
    }

    void logBytes(const char *prefix, const barray & data);

private:
    void flush();

#ifdef _DEBUG
    std::ofstream   logfile_;
#endif

    bool        authenticated_;
    bool        authenticatedHalf_;

    barray      login_;
    barray      authKey_;           // auth key
    RC4Encoder  encoder_;           // шифрование исходящих пакетов
    RC4Encoder  decoder_;           // шифрование входящих пакетов
    MPPCEncoder mppcEncoder_;

    barray      writeQ_;            // на тот случай, когда данные начинают приходить раньше, чем авторизуется клиент - то есть если серверное соединение успевает авторизоваться раньше
};


#endif