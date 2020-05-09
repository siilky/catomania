#include "stdafx.h"

#include "game/gtime.h"


void GTimer::tick()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    _LARGE_INTEGER li;
    li.HighPart = ft.dwHighDateTime;
    li.LowPart = ft.dwLowDateTime;

    Timestamp now = li.QuadPart / 10000;

    long long dt = now - tLast_;
    tLast_ = now;

    if (tCurrent_ == 0 || tLast_ == 0)
    {
        tCurrent_ = now;
        return;
    }

    if (dt < 0)
    {
        tCurrent_++;
        tServerCurrent_++;
        return;
    }

    tCurrent_ += dt;

    if (tServerCurrent_ != 0)
    {
        tServerCurrent_ += dt;
    }
}
