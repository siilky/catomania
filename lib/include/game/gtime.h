#ifndef gtime_h
#define gtime_h

#include "types.h"

#pragma warning(disable : 4505)     // 'GTime::tsToLocalSystem' : unreferenced local function has been removed

class GTimer
{
    public:
        GTimer()
            : tCurrent_(0)
            , tServerCurrent_(0)
            , tLast_(0)
            , tServerTz_(0)
        {}

        void tick();

        // get current linear timestamp in milliseconds
        Timestamp time() const
        {
            return tCurrent_;
        }

        // UTC
        Timestamp serverTime() const    
        {
            return tServerCurrent_;
        }

        Timestamp serverLocalTime() const
        {
            return tServerCurrent_ - tServerTz_ * 60 * 1000;
        }

        // sync local time
        void sync(Timestamp t)
        {
            tCurrent_ = t;
            tLast_ = t;
        }

        // sync server time
        void serverSync(Timestamp t)
        {
            tServerCurrent_ = t;
        }
        void serverSync(Timestamp t, int tz)
        {
            tServerCurrent_ = t;
            tServerTz_ = tz;
        }

    private:
        Timestamp   tCurrent_;
        Timestamp   tServerCurrent_;
        Timestamp   tServerTz_;         // mins
        Timestamp   tLast_;
};

namespace GTime
{
/*

    extern GTimer *gTimer;

    inline void tick()
    {
        gTimer->tick();
    }

    inline void sync(Timestamp t)
    {
        gTimer->sync(t);
    }

    inline void gameSync(Timestamp t)
    {
        gTimer->serverSync(t);
    }

    inline void gameSync(Timestamp t, int tz)
    {
        gTimer->serverSync(t, tz);
    }

    inline Timestamp time()
    {
        return gTimer->time();
    }

    inline Timestamp gameTime()
    {
        return gTimer->serverTime();
    }

    inline Timestamp gameTimeAsLocal()
    {
        return gTimer->serverTimeAsLocal();
    }
    */

    static inline Timestamp unixToTs(DWORD unixTime)
    {
        return unixTime == 0 ? 0 : (Timestamp(unixTime) * 1000 + 11644473600000LL);
    }

    static inline DWORD tsToUnix(Timestamp timeStamp)
    {
        return timeStamp == 0 ? 0 : DWORD((timeStamp - 11644473600000LL) / 1000);
    }

    static inline FILETIME tsToFileTime(Timestamp timeStamp)
    {
        ULARGE_INTEGER ul;
        ul.QuadPart = timeStamp * 10000 /*to system ns*/;

        FILETIME utc;
        utc.dwHighDateTime = ul.HighPart;
        utc.dwLowDateTime  = ul.LowPart;

        return utc;
    }

    static bool tsToSystem(Timestamp timeStamp, SYSTEMTIME *st)
    {
        assert(st != NULL);
        if (st == NULL)
        {
            return false;
        }

        FILETIME utc = tsToFileTime(timeStamp);
        if (FileTimeToSystemTime(&utc, st))
        {
            return true;
        }

        return false;
    }

    static bool tsToLocalSystem(Timestamp timeStamp, SYSTEMTIME *st)
    {
        assert(st != NULL);
        if (st == NULL)
        {
            return false;
        }

        FILETIME utc = tsToFileTime(timeStamp);
        FILETIME local;
        if (FileTimeToLocalFileTime(&utc, &local)
            && FileTimeToSystemTime(&local, st))
        {
            return true;
        }

        return false;
    }
};

#endif
