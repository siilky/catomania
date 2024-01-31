
#include "stdafx.h"

#include <cctype>

#include "log.h"
#include "md5.h"
#include "netio/connector.h"
#include "netdata/fragments.h"
#include "netdata/serialize.h"
#include "netdata/fragments_client.h"
#include "error.h"
#include "compactuint.h"

Connector::Connector()
{
#ifdef _DEBUG
    logfile_.open("binary.log", std::ios::trunc);
    if (logfile_)
    {
        logfile_.fill('0');
        logfile_.setf(std::ios::uppercase);
    }
#endif

#if defined (PW_SERVER_COMEBACK)
    std::ifstream file("comeback.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            comebackData_.push_back(line);
        }
    }
    else
        Log("Failed to read comeback.txt");

#endif
}

bool Connector::setAuth( const std::string    & login
                       , const std::string    & password
                       , bool                   forced /*= false*/)
{
    login_ = login;
    password_ = password;
    forcedLogin_ = forced;
    aKey.clear();
#if defined(PW_SERVER_TOKEN_2_AUTH)
    aKey.insert(aKey.end(), password.begin(), password.end());
#endif

    std::transform(login_.begin(), login_.end(), login_.begin(), [](unsigned char c) { return (unsigned char)std::tolower(c); });
    Log("Login:%hs", login_.c_str());
    return true;
}

bool Connector::setPreEncoding(int clientOption, const barray & clientKey,
                               int serverOption, const barray & serverKey)
{
    if (clientOption != 2 || serverOption != 2)
    {
        fail(L"Unsupported encoding type");
        return false;
    }

    clientPreEnc_.init(clientKey);
    serverPreEnc_.init(serverKey);
    doPreEndoding_ = true;
    return true;
}

barray Connector::processDataIn(const barray & data$)
{
    keepaliveMisses_ = 0;

    if (authenticated_)
    {
        barray out(data$);
#if !defined(PW_SERVER_COMEBACK)
        serverEnc.encode(out);
#endif

        // tofstream file("io.data", ios_base::out | ios_base::app);
        // file << '[' << hex << setw(8) << time(0) << "]  " << endl;
        // dumpBytes(file, out.begin(), out.end());

        barray outUnpacked = unpacker_.decompress(out);
        //barray outUnpacked2 = mppcDecoder_.transform(out);
        //assert(outUnpacked == outUnpacked2);

        logBytes("<==", outUnpacked);
        return outUnpacked;
    }
    else
    {
        barray data(data$);
        if (doPreEndoding_)
        {
            serverPreEnc_.encode(data);
        }

        logBytes("<==", data);

        barray::const_iterator p(data.begin());
        Fragment f(data, p);
        if (!f.isOk())
        {
            fail(L"Data error");
            return barray();
        }

        const barray & bytes = f.bytes();

        switch (f.getId())
        {
            case 0x01:      // server key
            {
                // CECGameSession::OnPrtcChallenge

                clientdata::FragmentChallengeResponse fr;

                fr.login = login_;

            #if defined(PW_SERVER_TOKEN_AUTH)
                // ����� ���� �� ���������� ������
                unsigned keyLength = bytes[0];  // should be cu
                if (keyLength > bytes.size() - 1)
                {
                    fail(L"[01] Key size exceeds packet length");
                    break;
                }

                barray key(bytes.begin() + 1, bytes.begin() + 1 + keyLength);
                key.insert(key.begin(), password_.begin(), password_.end());
                aKey = MD5Bin(key);

                fr.response = aKey;

            #elif defined(PW_SERVER_TOKEN_2_AUTH)
                fr.response = aKey;

            #else
                // �������� MD5 �� login+password
                string lpw(login_ + password_);
                barray md5LogPsw = MD5Bin(barray(lpw.begin(), lpw.end()));

                // Octets nonce
                unsigned keyLength = bytes[0];        
                if (keyLength > bytes.size() - 1)
                {
                    fail(L"[01] Key size exceeds packet length");
                    break;
                }

                // version, algo
                auto offset = 1 + keyLength;
                Log("Challenge version: %i.%i.%i.%i algo %i", (int)bytes[offset], (int)bytes[offset + 1], (int)bytes[offset + 2], (int)bytes[offset + 3], (int)bytes[offset + 4]);
                
                barray key(bytes.begin() + 1, bytes.begin() + 1 + keyLength);

                // �������� MD5_HMAC �� md5LogPsw � key
                aKey = HMAC_MD5(md5LogPsw, key);

                fr.response = aKey;
            #endif

#if defined(PW_SERVER_TOKEN_AUTH)
                fr.use_token = 1;
#elif defined(PW_SERVER_TOKEN_2_AUTH)
                fr.use_token = 2;
#else
                fr.use_token = 0;
#endif

#if defined(PW_SERVER_COMEBACK)
                if (comebackData_.empty())
                {
                    fail(L"No comeback data");
                    break;
                }

                auto xorString = [] (const std::string & input) -> std::string
                {
                    std::string output;
                    const unsigned char kb[] = { 0x83, 0x23, 0x90, 0x13 };
                    for (size_t i = 0; i < input.size(); i++)
                    {
                        output += input[i] ^ kb[i % 4];
                    }
                    return output;
                };

                // here should be utf8 conversion from wstring if we have unicode file
                
                fr.hwid = xorString(comebackData_[0]);
                for (size_t i = 1; i < comebackData_.size(); i++)
                {
                    auto hdd_id = xorString(comebackData_[i]);
                    fr.hdd_ids.push_back(hdd_id);
                }
#endif

                auto bytes_re = fr.assemble();

                Log("Sending login");
                sendReply(processDataOut(bytes_re));

                break;
            }

            case 0x02:      // key exchange completed
            {
                // ����� ���� �� ���������� ������
                unsigned keyLength = bytes[0];        // not sure if CU is used here
                if (keyLength > bytes.size() - 1)
                {
                    fail(L"[02] Key size exceeds packet length");
                    break;
                }

                barray recvKey(bytes.begin() + 1, bytes.begin() + 1 + keyLength);

                //������� MD5_HMAC �� login(����) � aKey+recvKey(�����)

                barray sumkey(aKey);
                sumkey.insert(sumkey.end(), recvKey.begin(), recvKey.end());

                barray clientKey = HMAC_MD5(barray(login_.begin(), login_.end()), sumkey);
                clientEnc.init(clientKey);

                barray randomKey(16);
                for (unsigned int i = 0; i < 16; i++)
                {
                    randomKey[i] = byte(rand() % 0xFF);
                }

                // �������� MD5_HMAC �� login(����) � aKey+randomKey(�����)

                sumkey = aKey;
                sumkey.insert(sumkey.end(), randomKey.begin(), randomKey.end());

                barray serverKey = HMAC_MD5(barray(login_.begin(), login_.end()), sumkey);
                serverEnc.init(serverKey);

                authenticated_ = true;

                barray pack(3);
                pack[0] = 0x02;                             // ������� ������
                pack[1] = 0x12;                             // ����� ������
                pack[2] = byte(randomKey.size());           // ����� �����
                pack.insert(pack.end(), randomKey.begin(), randomKey.end());  //��������� ����
                pack.push_back(forcedLogin_ ? 0x01 : 0);    // ���� "������� ��������"

                Log("sending seed");
                sendReply(processDataOut(pack));

                break;
            }

            case 0x05:      // login error
            {
            #if PW_SERVER_VERSION >= 1500
                int code = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3] << 0);
            #else
                int code = bytes[0];
            #endif
                fail(ServerError::getString(code + 10150));
                break;
            }

            default:
                fail(L"Unexpected login sequence");
                return data;
        }

        return barray();
    }
}

barray Connector::processDataOut(const barray & data)
{
    logBytes("==>", data);

    if (authenticated_)
    {
        barray encoded(data);
#if !defined(PW_SERVER_COMEBACK)
        clientEnc.encode(encoded);
#endif
        return encoded;
    }
    else
    {
        if (doPreEndoding_)
        {
            barray encoded(data);
            clientPreEnc_.encode(encoded);
            return encoded;
        }
        else
        {
            return data;
        }
    }
}

void Connector::processTimer()
{
// Turn off keepalive detection cause sometimes it breaks connection seemed to be good
//     keepaliveMisses_ ++;
//     if (keepaliveMisses_ > MAX_SERVR_KEEPALIVE)
//     {
//         fail(L"Keepalive timed out");
//     }
//     else
    {
        barray keepalive(3);
        keepalive[0] = 0x5A;
        keepalive[1] = 0x01;
        keepalive[2] = 0x5A;

        barray out(keepalive);
        logBytes("==>", out);
        clientEnc.encode(out);
        sendReply(out);
    }
}

void Connector::logBytes(const char *prefix, const barray & data)
{
#ifdef _DEBUG
    if ( ! logfile_)
    {
        return;
    }

    #pragma warning(suppress:28159)   // C28159: Consider using 'GetTickCount64' instead of 'GetTickCount'
    logfile_ << prefix << "[" << hex << setw(8) << time(0) << '.' << GetTickCount() << "] L=" << dec << data.size() << endl 
             << data << endl;
#endif
}

//

ConnectorServerEp::ConnectorServerEp()
    : authenticated_(false)
    , authenticatedHalf_(false)
{
#ifdef _DEBUG
    logfile_.open("binary_sep.log", std::ios::trunc);
    if (logfile_)
    {
        logfile_.fill('0');
        logfile_.setf(std::ios::uppercase);
    }
#endif
}

barray ConnectorServerEp::processDataIn(const barray & in)
{
    // from client

    barray data(in);

    if (authenticatedHalf_)
    {
        decoder_.encode(data);
    }

    logBytes("==>", data);

    if (authenticated_)
    {
        return data;
    }

    barray::const_iterator p(data.begin());
    Fragment f(data, p);
    if (!f.isOk())
    {
        fail(L"Data error");
        return barray();
    }

    const barray & bytes = f.bytes();
    if (bytes.size() < 2)
    {
        fail(L"Data error");
        return barray();
    }

    switch (f.getId())
    {
        case 03:
        {
            // �������� ���� ������� � ������������ ���������� �������

            int offset = 0;
            int loginLength = bytes[offset++];
            login_ = barray(bytes.begin() + offset, bytes.begin() + offset + loginLength);

            offset += loginLength;
            int authKeyLength = bytes[offset++];
            authKey_ = barray(bytes.begin() + offset, bytes.begin() + offset + authKeyLength);

            barray randomKey;
            for (int i = 0; i < 16; i++)
            {
                randomKey.push_back((rand() % 0xFF));
            }

            barray sumkey(authKey_);
            sumkey.insert(sumkey.end(), randomKey.begin(), randomKey.end());

            barray clientKey = HMAC_MD5(login_, sumkey);
            decoder_.init(clientKey);

            // send key to client
            // [02] L Key

            barray pack(3);
            pack[0] = 0x02;                             // ������� ������
            pack[1] = byte(randomKey.size()) + 2;       // ����� ������
            pack[2] = byte(randomKey.size());           // ����� �����
            pack.insert(pack.end(), randomKey.begin(), randomKey.end());
            pack.push_back(0);                          // ���� "������� ��������" ��� �������

            Log("sending client seed");
            logBytes("<==", pack);
            sendReply(pack);

            authenticatedHalf_ = true;
            break;
        }

        case 02:
        {
            // �������� ���� ������� � �������������� �������
            int keyLength = bytes[0];
            barray cKey(bytes.begin() + 1, bytes.begin() + 1 + keyLength);
            cKey.insert(cKey.begin(), authKey_.begin(), authKey_.end());
            barray serverKey = HMAC_MD5(login_, cKey);
            encoder_.init(serverKey);

            authenticated_ = true;
            flush();

            break;
        }

        default:
            fail(L"Unexpected login sequence");
            return data;
    }

    return barray();
}

barray ConnectorServerEp::processDataOut(const barray & data)
{
    // to client / from server

    if ( ! authenticated_)
    {
        writeQ_.insert(writeQ_.end(), data.begin(), data.end());
        return barray();
    }

    logBytes("<==", data);

    barray out = mppcEncoder_.transform(data);
    // logBytes("<==[Z]", out);
    encoder_.encode(out);
    // logBytes("<==[X]", out);
    return out;
}

void ConnectorServerEp::logBytes(const char *prefix, const barray & data)
{
#ifdef _DEBUG
        if (!logfile_)
        {
            return;
        }

#pragma warning(suppress:28159)   // C28159: Consider using 'GetTickCount64' instead of 'GetTickCount'
        logfile_ << prefix << "[" << hex << setw(8) << time(0) << '.' << GetTickCount() << "] L=" << dec << data.size() << endl
            << data << endl;
#endif
}

void ConnectorServerEp::flush()
{
    if (!writeQ_.empty() && authenticated_)
    {
        sendReply(processDataOut(writeQ_));
        writeQ_.clear();
    }
}
