
#include "stdafx.h"

#include "common.h"
#include "persistence2.h"

static std::string UTF8FromUTF16(const std::wstring & utf16) 
{ 
    // Special case of empty input string 
    if (utf16.empty()) return std::string(); 

    std::string utf8;
    utf8.resize(utf16.size() * 2); 

    int utf8Length;
    do
    {
        utf8Length = ::WideCharToMultiByte( CP_UTF8, 0
                                          , utf16.data(), utf16.length()
                                          , &utf8[0], utf8.length()
                                          , NULL, NULL);
        if (utf8Length == 0)
        {
            DWORD error = ::GetLastError(); 
            if (error == ERROR_INSUFFICIENT_BUFFER)
            {
                utf8.resize(utf8.length() * 2); 
            }
            else
            {
                Log("WideCharToMultiByte failed with %i", error);
                return std::string();
            }
        }
    }
    while (utf8Length == 0);

    utf8.resize(utf8Length);
    return utf8; 
} 

static std::wstring UTF16FromUTF8(const std::string & utf8) 
{ 
    if (utf8.empty()) return std::wstring(); 

    std::wstring utf16;
    utf16.resize(utf8.size() * 2); 

    int utf16Length;
    do
    {
        // Fail if an invalid input character is encountered 
        utf16Length = ::MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS
                                           , utf8.data(), utf8.length()
                                           , &utf16[0], utf16.length());
        if (utf16Length == 0)
        {
            DWORD error = ::GetLastError(); 
            if (error == ERROR_INSUFFICIENT_BUFFER)
            {
                utf16.resize(utf16.length() * 2); 
            }
            else
            {
                Log("WideCharToMultiByte failed with %i", error);
                return std::wstring();
            }
        }
    }
    while (utf16Length == 0);

    utf16.resize(utf16Length);
    return utf16; 
} 

static bool saveFile(const std::wstring & string, const std::wstring & filename, bool doBackup)
{
    assert( ! filename.empty());
    bool result = false;

    std::string utf8 = UTF8FromUTF16(string);

    std::wstring tmpName(filename);
    if (doBackup)
    {
        tmpName += L".n";
    }

    HANDLE hFile = CreateFileW(tmpName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD written;
        if (WriteFile(hFile, &utf8[0], utf8.size(), &written, NULL)
            && written == utf8.size())
        {
            result = true;
        }
        else
        {
            Log("Failed to write new config");
            assert(0);
        }

        CloseHandle(hFile);
    }
    if (result && doBackup)
    {
        if (MoveFileExW(filename.c_str(), (filename + L".o").c_str(), MOVEFILE_REPLACE_EXISTING) == 0)
        {
            // may not exist
            Log("Move previous config failed");
        }

        if (MoveFileExW(tmpName.c_str(), filename.c_str(), MOVEFILE_REPLACE_EXISTING) == 0)
        {
            Log("Move new config failed");
            assert(0);
        }
    }

    return result;
}

static bool loadFile(std::wstring & string, const std::wstring & filename)
{
    assert( ! filename.empty());

    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Log("Failed to open config file %s", filename.c_str());
        return false;
    }
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE)
    {
        Log("Failed to open config file %s", filename.c_str());
        return false;
    }

    bool result = true;

    if (fileSize > 0)
    {
        std::string utf8;
        utf8.resize(fileSize);

        DWORD bytesRead = 0;
        if (ReadFile(hFile, &utf8[0], fileSize, &bytesRead, NULL) == FALSE)
        {
            result = false;
        }
        else
        {
            assert(bytesRead == fileSize);

            string = UTF16FromUTF8(utf8);

            // remove BOM
            if (string.size() > 0
                && string[0] == 0xFEFF)
            {
                string[0] = L' ';
            }
        }
    }

    CloseHandle(hFile);
    return result;
}

//

Persistence::~Persistence()
{
    save();
}

bool Persistence::open(const std::wstring & filename, bool & restored)
{
    isRestored_ = restored = false;

    if (fileName_ == filename)
    {
        // already opened
        return true;
    }

    fileCache_.clear();

    bool result = loadFile(fileCache_, filename);
    result &= json::read(fileCache_, jsonRoot_);
    if (!result)
    {
        isRestored_ = restored = true;

        result = loadFile(fileCache_, filename + L".n");
        result &= json::read(fileCache_, jsonRoot_);
        if (!result)
        {
            result = loadFile(fileCache_, filename + L".o");
            result &= json::read(fileCache_, jsonRoot_);
        }
    }
    if (result)
    {
        fileName_ = filename;
    }
    else
    {
        fileCache_.clear();
    }

    return result;
}

void Persistence::save()
{
    if (fileName_.empty())
    {
        // not opened
        return;
    }

    std::wostringstream stream;
    json::write_formatted(jsonRoot_, stream);

    std::wstring str = stream.str();
    if ( ! isRestored_
        && str == fileCache_)
    {
        return;
    }

    if (saveFile(str, fileName_, true))
    {
        fileCache_ = str;
        isRestored_ = false;
    }
}

void JsonValue::save()
{
    if (container_ != NULL)
    {
        container_->save();
    }
}

void JsonValue::saveTo(const std::wstring & filename, bool doBackup)
{
    std::wostringstream stream;
    json::write_formatted(*this, stream);
    saveFile(stream.str(), filename, doBackup);
}

bool JsonValue::loadFrom(const std::wstring & filename)
{
    std::wstring str;
    if (loadFile(str, filename))
    {
        return json::read(str, *this);
    }
    return false;
}

std::wstring JsonValue::string() const
{
    return json::write(*this);
}

bool JsonValue::read(const std::wstring & string)
{
    return json::read(string, *this);
}
