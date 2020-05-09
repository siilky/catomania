#ifndef encoder_h_
#define encoder_h_

#include "types.h"


class RC4Encoder
{
public:
    void init(const barray & key);
    void encode(barray & data);

    #if defined _DEBUG
        static bool test();
    #endif

private:
    byte matrix[256];
    byte offset;
    byte offset2;
};


#endif