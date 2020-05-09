// $Id: netchannelx.cpp 803 2012-05-10 19:57:20Z jerry $
//
#include "stdafx.h"

#include "common.h"
#include "io/pipe.h"
#include "netio/netchannel.h"
#include "netio/netchannelx.h"


//------------------------------------------------------------------------------
NetChannelX::NetChannelX(NetChannelXType type)
    : Pipe(PIPE_MODE_CLIENT)
    , type_(type)
{
}

bool NetChannelX::connect(DWORD pid)
{
    tstring pipeName(_T("pwnetio"));
    TCHAR   pid_s[9];
    if (_itot_s(pid, pid_s, sizeof_array(pid_s), 16) != 0)
    {
        throw eCrtError;
    }
    pipeName += pid_s;

    if (type_ == CHANNEL_TYPE_SERVER)
    {
        pipeName += _T("_S");
    }
    else
    {
        pipeName += _T("_C");
    }

    return Pipe::connect(pipeName);
}
