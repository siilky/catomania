// MD5.H - header file for MD5.CPP

/*
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
*/

#ifndef __MD5_H
#define __MD5_H

#include "common.h"

extern "C" {

typedef unsigned long DWORD;

// MD5 context.
typedef struct
{
	DWORD   state[4];           // state (ABCD)
	DWORD   count[2];           // number of bits, modulo 2^64 (lsb first)
	byte    buffer[64];         // input buffer
} MD5_CTX;

// MD5 initialization. Begins an MD5 operation, writing a new context.
void MD5Init(MD5_CTX *context);

// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block, and updating the
// context.
void MD5Update(MD5_CTX *context, const char *input_, unsigned int inputLen);

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
void MD5Final(byte digest[16], MD5_CTX *context);

void digest2ascii(char *ascii, const byte *digest);
void ascii2digest(byte *digest, const char *ascii);

} // extern "C"

class MD5
{
public:
	MD5();
	void Init();
	void Update(const barray & data);
	barray Final(); // return digest

//private:
	MD5_CTX _ctx;
};

std::string MD5Hex(const barray & data);
barray MD5Bin(const barray & data);
barray HMAC_MD5(const barray & key, const barray & data);    // returns digest


#endif//__MD5_H