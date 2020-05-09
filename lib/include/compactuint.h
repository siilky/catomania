// $Id: compactuint.h 935 2013-05-12 14:07:16Z jerry $
#ifndef compactuint_h
#define compactuint_h

// The compactUint is:
// 0 ... 79 8080 8081 ... BFFF C0004000 ... DFFFFFFF E020000000
//

// Exception is thrown if incomplete compact uint discovered or 
// read attempt on end of stream

// Get compact uint value from the stream.
// If succeeds: advances position iterator to the end of uint read and returns true
// if there is no data to read full uint - returns false
bool getCUI_safe(const barray & data, barray::const_iterator & position, __notnull unsigned int *value);

// Get compact uint value from the stream.
// If succeeds: advances position iterator to the end of uint read
// if there is no data to read full uint - throws exception
unsigned int getCUI(const barray &data, barray::const_iterator & position);

// Get compact uint value from the stream.
// If succeeds: position iterator will be unchanged and returns true
// if there is no data to read full uint - returns false
bool peekCUI_safe(const barray &data, barray::const_iterator position, __notnull unsigned int *value);

// Get compact uint value from the stream.
// If succeeds: position will be unchanged
// if there is no data to read full uint - throws exception
unsigned int peekCUI(const barray &data, barray::const_iterator position);

// Put compact uint value to the stream.
// returns number of bytes put
unsigned putCUI(barray::iterator & position, unsigned int val);

// Get compact uint converted bytes size.
unsigned getPackedCUISize(unsigned int val);

#endif // compactuint_h