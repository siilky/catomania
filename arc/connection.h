#pragma once

#include "qlib\netio\qchanneltcp.h"
#include "netdata\connection.h"

namespace ARC
{

class FragmentProcessor : public ::FragmentProcessor
{
public:
    void setTeaEncoder(const std::array<uint32_t, 4> & key)
    {
        teaKey_ = key;
        doEncode_ = true;
    }

    void process(FragmentBase *fragment) override;

private:
    bool                    doEncode_ = false;
    std::array<uint32_t, 4> teaKey_;
};


class Connection : public QObject,
                   public ::ConnectionBase
{
    Q_OBJECT
public:
    Connection();
    virtual ~Connection() override;

    bool open(const std::wstring & adddress, const QNetworkProxy &proxy);
    void close();

    void setTeaKey(const std::array<uint32_t, 4> & key);

Q_SIGNALS:
    void connected();
    void disconnected();

private Q_SLOTS:
    void onDataIn(const QByteArray & data);
};


}
