#ifndef portmanager_h
#define portmanager_h



class PortManager
{
public:
    static PortManager * instance();

    quint16 acquire();  // return 0 if no available
    void release(quint16 port);

private:
    static const quint16 FirstAvailable = 30000;
    static const quint16 LastAvailable = 60000;

    std::set<quint16>   ports_;

    PortManager();
    ~PortManager();
};

class UniquePort
{
public:
    UniquePort();
    ~UniquePort();

    quint16 value() const
    {
        assert(value_ != 0);
        return value_;
    }

private:
    quint16 value_;
};

#endif
