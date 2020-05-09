///////////////////////////////////////////////////////////////////
//                 Ini.h
//
// "CIni" is a simple API wrap class used for ini file access.
// The purpose of this class is to make ini file access more
// convenient than direct API calls.
//	
// This file is distributed "as is" and without any expressed or implied
// warranties. The author holds no responsibilities for any possible damages
// or loss of data that are caused by use of this file. The user must assume
// the entire risk of using this file.
//
// 7/08/2002    Bin Liu
//
// Update history:
//
//  7/08/2002 -- Initial release.
//  7/14/2002 -- Added "IncreaseInt" and "AppendString"
//  9/02/2002 -- Added "removeProfileSection" and "RemoveProfileEntry"
//  2/09/2003 -- The class has been made unicode-compliant
// 11/04/2003 -- Integrated MFC support, added in new member functions
//               for accessing arrays.
// 11/08/2003 -- Fixed "GetString" and "GetPathName" method, changed parameter
//               from "LPSTR" to "LPTSTR"
// 11/10/2003 -- Renamed method "GetKeys" to "GetKeyLines",
//               Added method "GetKeyNames"
//               Added parameter "bTrimString" to method "GetArray"
// 11/14/2003 -- Use "__AFXWIN_H__" instead of "_AFXDLL" to determine MFC presence
//               Removed length limit on "m_pszPathName"
//               Removed "GetStruct" and "WriteStruct"
//               Added "GetDataBlock", "WriteDataBlock", "AppendDataBlock"
//               Added "GetChar" and "WriteChar"
//
///////////////////////////////////////////////////////////////////

#ifndef __INI_H__
#define __INI_H__


// If MFC is linked, we will use CStringArray for great convenience
#ifdef __AFXWIN_H__
	#include <afxtempl.h>
#endif

// Number bases
#define BASE_BINARY			2
#define BASE_OCTAL			8
#define BASE_DECIMAL		10
#define BASE_HEXADECIMAL	16

//---------------------------------------------------------------
//	    Callback Function Type Definition
//---------------------------------------------------------------
// The callback function used for parsing a "double-null terminated string".
// When called, the 1st parameter passed in will store the newly extracted sub
// string, the 2nd parameter is a 32-bit user defined data, this parameter can
// be NULL. The parsing will terminate if this function returns zero. To use
// the callback, function pointer needs to be passed to "CIni::ParseDNTString".
typedef bool (CALLBACK *SUBSTRPROC)(LPCTSTR, LPVOID);

class CIni
{
public:		

	//-----------------------------------------------------------
	//    Constructors & Destructor
	//-----------------------------------------------------------
	CIni(); // Default constructor
	CIni(LPCTSTR lpPathName); // Construct with a given file name
	virtual ~CIni();

	//-----------------------------------------------------------
	//    Ini File Path Name Access
	//-----------------------------------------------------------
	void SetPathName(LPCTSTR lpPathName); // Specify a new file name
	DWORD GetPathName(LPTSTR lpBuffer, DWORD dwBufSize) const; // Retrieve current file name
#ifdef __AFXWIN_H__
	CString GetPathName() const;
#endif
	
	//------------------------------------------------------------
	//    String Access
	//------------------------------------------------------------	
    tstring GetString(LPCTSTR lpSection, LPCTSTR lpKey, tstring & defaultStr) const;
    tstring GetString(LPCTSTR lpSection, LPCTSTR lpKey) const;
#ifdef __AFXWIN_H__
	CString GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault = NULL) const;
#endif
	bool WriteString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue) const;

	// Read a string from the ini file, append it with another string then write it
	// back to the ini file.
	bool AppendString(LPCTSTR Section, LPCTSTR lpKey, LPCTSTR lpString) const;
	
	//------------------------------------------------------------
	//    Ini File String Array Access
	//------------------------------------------------------------	
	// Parse the string retrieved from the ini file and split it into a set of sub strings.
	DWORD GetArray(LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDelimiter = NULL, bool bTrimString = TRUE) const;
#ifdef __AFXWIN_H__
	void GetArray(LPCTSTR lpSection, LPCTSTR lpKey, CStringArray* pArray, LPCTSTR lpDelimiter = NULL, bool bTrimString = TRUE) const;
	bool WriteArray(LPCTSTR lpSection, LPCTSTR lpKey, const CStringArray* pArray, int nWriteCount = -1, LPCTSTR lpDelimiter = NULL) const;
#endif	
	
	//------------------------------------------------------------
	//    Primitive Data Type Access
	//------------------------------------------------------------
	int GetInt(LPCTSTR lpSection, LPCTSTR lpKey, int nDefault, int nBase = BASE_DECIMAL) const;
	bool WriteInt(LPCTSTR lpSection, LPCTSTR lpKey, int nValue, int nBase = BASE_DECIMAL) const;
	bool IncreaseInt(LPCTSTR lpSection, LPCTSTR lpKey, int nIncrease = 1, int nBase = BASE_DECIMAL) const;
	
	UINT GetUInt(LPCTSTR lpSection, LPCTSTR lpKey, UINT nDefault, int nBase = BASE_DECIMAL) const;
	bool WriteUInt(LPCTSTR lpSection, LPCTSTR lpKey, UINT nValue, int nBase = BASE_DECIMAL) const;
	bool IncreaseUInt(LPCTSTR lpSection, LPCTSTR lpKey, UINT nIncrease = 1, int nBase = BASE_DECIMAL) const;
	
	bool GetBool(LPCTSTR lpSection, LPCTSTR lpKey, bool bDefault) const;
	bool WriteBool(LPCTSTR lpSection, LPCTSTR lpKey, bool bValue) const;
	bool InvertBool(LPCTSTR lpSection, LPCTSTR lpKey) const;
	
	double GetDouble(LPCTSTR lpSection, LPCTSTR lpKey, double fDefault) const;
	bool WriteDouble(LPCTSTR lpSection, LPCTSTR lpKey, double fValue, int nPrecision = -1) const;
	bool IncreaseDouble(LPCTSTR lpSection, LPCTSTR lpKey, double fIncrease, int nPrecision = -1) const;

	TCHAR GetChar(LPCTSTR lpSection, LPCTSTR lpKey, TCHAR cDefault) const;
	bool WriteChar(LPCTSTR lpSection, LPCTSTR lpKey, TCHAR c) const;

	//------------------------------------------------------------
	//    User-Defined Data Type & Data Block Access
	//------------------------------------------------------------
	POINT GetPoint(LPCTSTR lpSection, LPCTSTR lpKey, POINT ptDefault) const;
	bool WritePoint(LPCTSTR lpSection, LPCTSTR lpKey, POINT pt) const;
	
	RECT GetRect(LPCTSTR lpSection, LPCTSTR lpKey, RECT rcDefault) const;
	bool WriteRect(LPCTSTR lpSection, LPCTSTR lpKey, RECT rc) const;

	DWORD GetDataBlock(LPCTSTR lpSection, LPCTSTR lpKey, LPVOID lpBuffer, DWORD dwBufSize, DWORD dwOffset = 0) const;
	bool WriteDataBlock(LPCTSTR lpSection, LPCTSTR lpKey, LPCVOID lpData, DWORD dwDataSize) const;
	bool AppendDataBlock(LPCTSTR lpSection, LPCTSTR lpKey, LPCVOID lpData, DWORD dwDataSize) const;
	
	//------------------------------------------------------------
	//    Section Operations
	//------------------------------------------------------------
	bool IsSectionExist(LPCTSTR lpSection) const;
	DWORD GetSectionNames(LPTSTR lpBuffer, DWORD dwBufSize) const;
#ifdef __AFXWIN_H__
	void GetSectionNames(CStringArray* pArray) const;
#endif
	bool CopySection(LPCTSTR lpSrcSection, LPCTSTR lpDestSection, bool bFailIfExist) const;
	bool MoveSection(LPCTSTR lpSrcSection, LPCTSTR lpDestSection, bool bFailIfExist = TRUE) const;
	bool DeleteSection(LPCTSTR lpSection) const;
	
	//------------------------------------------------------------
	//    Key Operations
	//------------------------------------------------------------
	bool IsKeyExist(LPCTSTR lpSection, LPCTSTR lpKey) const;	
	DWORD GetKeyLines(LPCTSTR lpSection, LPTSTR lpBuffer, DWORD dwBufSize) const;
#ifdef __AFXWIN_H__
	void GetKeyLines(LPCTSTR lpSection, CStringArray* pArray) const;
#endif
	DWORD GetKeyNames(LPCTSTR lpSection, LPTSTR lpBuffer, DWORD dwBufSize) const;
#ifdef __AFXWIN_H__	
	void GetKeyNames(LPCTSTR lpSection, CStringArray* pArray) const;
#endif
	bool CopyKey(LPCTSTR lpSrcSection, LPCTSTR lpSrcKey, LPCTSTR lpDestSection, LPCTSTR lpDestKey, bool bFailIfExist) const;
	bool MoveKey(LPCTSTR lpSrcSection, LPCTSTR lpSrcKey, LPCTSTR lpDestSection, LPCTSTR lpDestKey, bool bFailIfExist = TRUE) const;
	bool DeleteKey(LPCTSTR lpSection, LPCTSTR lpKey) const;

	//------------------------------------------------------------
	// Parse a "Double-Null Terminated String"
	//------------------------------------------------------------
	static bool ParseDNTString(LPCTSTR lpString, SUBSTRPROC lpFnStrProc, LPVOID lpParam = NULL);

	//------------------------------------------------------------
	// Check for Whether a String Representing TRUE or FALSE
	//------------------------------------------------------------
	static bool StringToBool(LPCTSTR lpString, bool bDefault = FALSE);
		
protected:

    DWORD GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDefault = NULL) const;

	//------------------------------------------------------------
	//    Helper Functions
	//------------------------------------------------------------
	static LPTSTR __StrDupEx(LPCTSTR lpStart, LPCTSTR lpEnd);
	static bool __TrimString(LPTSTR lpBuffer);
	LPTSTR __GetStringDynamic(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault = NULL) const;
	static DWORD __StringSplit(LPCTSTR lpString, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDelimiter = NULL, bool bTrimString = TRUE);
	static void __ToBinaryString(UINT nNumber, LPTSTR lpBuffer, DWORD dwBufSize);
	static int __ValidateBase(int nBase);
	static void __IntToString(int nNumber, LPTSTR lpBuffer, int nBase);
	static void __UIntToString(UINT nNumber, LPTSTR lpBuffer, int nBase);
	static bool CALLBACK __SubStrCompare(LPCTSTR lpString1, LPVOID lpParam);
	static bool CALLBACK __KeyPairProc(LPCTSTR lpString, LPVOID lpParam);	
#ifdef __AFXWIN_H__
	static bool CALLBACK __SubStrAdd(LPCTSTR lpString, LPVOID lpParam);
#endif

	//------------------------------------------------------------
	//    Member Data
	//------------------------------------------------------------
	LPTSTR m_pszPathName; // Stores path of the associated ini file
};

#endif // #ifndef __INI_H__