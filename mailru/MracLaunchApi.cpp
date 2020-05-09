#include "stdafx.h"

#include "MracLaunchApi.h"


namespace PwMailRu
{

    static const wchar_t mracDll[] = L"mrac.dll";
    static const QLatin1String appFile("elementclient.exe");
    //static const QString appFile("app.exe");

    MracLaunchApi::MracLaunchApi(QString clientPath)
        : clientPath_(clientPath)
    {
        assert(!clientPath.isEmpty());

        if (!clientPath_.endsWith(L'\\')
            && !clientPath_.endsWith(L'/'))
        {
            clientPath_.append(L'\\');
        }
    }

    MracLaunchApi::~MracLaunchApi()
    {
        deleteLauncher();
        mrac_.unload();
    }

    bool MracLaunchApi::load()
    {
        mrac_.setFileName(clientPath_ + "mraclaunchapi.dll");

        createMracLauncher = (decltype(createMracLauncher))mrac_.resolve("CreateMracLauncher");
        if (createMracLauncher == nullptr)
        {
            lastError_ = GetLastError();
            qDebug() << "Failed to load/resolve createMracLauncher: " << mrac_.errorString();
            return false;
        }

        mracLauncher_ = (decltype(mracLauncher_))createMracLauncher(1);
        assert(mracLauncher_);

        lastError_ = 0;
        return true;
    }

    bool MracLaunchApi::launch(unsigned userId,
                               int(__cdecl *mracCallback)(void *ctx, wchar_t *launchExeFullPath, wchar_t *parameters, wchar_t *directory, void *hProcess),
                               int(__cdecl *mracProgressCallback)(void *ctx, int progress),
                               void *cbArg)
    {
        if (!mracLauncher_ || clientPath_.isEmpty())
        {
            return false;
        }

        bool r = false;

        uint32_t gameId = 61;
        r = mracLauncher_->setOption(mracLauncher_, 1, &gameId);
        if (!r)
        {
            qDebug() << "Failed to setOption 1";
            mracLauncher_->getResult(mracLauncher_, 6, &lastError_);
            return false;
        }

        auto persId = std::to_string(userId);
        uint32_t id = (uint32_t)persId.c_str();
        r = mracLauncher_->setOption(mracLauncher_, 2, &id);
        if (!r)
        {
            qDebug() << "Failed to setOption 2";
            mracLauncher_->getResult(mracLauncher_, 6, &lastError_);
            return false;
        }

        // "-job=2_392 -job_pipe=GameCenterV5_B5D2D6382CCE0B8C7789C29AF0F08B95 -job_hint=GCJobGameLaunch"
    //     const char pipeName[] = "GameCenterV5_B5D2D6382CCE0B8C7789C29AF0F08B95";
    //     r = mracLauncher_->setOption(mracLauncher_, 4, &id);
    //     if (!r)
    //     {
    //         qDebug() << "Failed to setOption 4";
    //         return false;
    //     }

    //     std::wstring exeName = exePath + L"elementclient.exe";
    //     auto protectExeFullPath = exeName.c_str();
    //     r = mracLauncher_->setOption(mracLauncher_, 5, (uint32_t*)&protectExeFullPath);
    //     if (!r)
    //     {
    //         qDebug() << "Failed to setOption 5";
    //         return false;
    //     }

        struct Callback
        {
            void *arg;
            void *callback;
        }
        cb{ cbArg, mracCallback };
        r = mracLauncher_->setOption(mracLauncher_, 3, (uint32_t*)&cb);
        if (!r)
        {
            qDebug() << "Failed to setOption 3";
            mracLauncher_->getResult(mracLauncher_, 6, &lastError_);
            return false;
        }

        struct Callback pcb { cbArg, mracProgressCallback };
        r = mracLauncher_->setOption(mracLauncher_, 8, (uint32_t*)&pcb);
        if (!r)
        {
            qDebug() << "Failed to setOption 8";
            mracLauncher_->getResult(mracLauncher_, 6, &lastError_);
            return false;
        }

        void* pHandle = 0;
        r = mracLauncher_->launch(mracLauncher_, (const wchar_t*)(clientPath_ + appFile).utf16(), L"", L"", (uint32_t*)&pHandle);
        if (!r)
        {
            qDebug() << "Failed to mrac launch";
            mracLauncher_->getResult(mracLauncher_, 6, &lastError_);
            return false;
        }

        lastError_ = 0;
        return true;
    }

    std::tuple<bool, HANDLE, HANDLE> MracLaunchApi::createProcess(unsigned userId,
                                                                  QString accId,
                                                                  QString nick,
                                                                  QString token,
                                                                  QString mracToken,
                                                                  bool isSuspended)
    {
        // GC_PROJECT_ID=61
        // GC_TYPE_ID=0
        // GC_PIPE_NAME=GameCenterV5_B5D2D6382CCE0B8C7789C29AF0F08B95
        // GC_HG64_LL=1981127712
        // GC_HG64_ET=2003215488
        // GC_USER_NAME=Furry TLK
        // GC_USER_NICK=Furry TLK
        // GC_USER_AVATAR=f:\Temp\CacheGameCenter\CurrentAvatar.png
        // GC_PERS_ID=196434
        // GC_MRAC_TOKEN=WbRm2k7Ggv7xwwniqyQFNgtxK2pUFJ2n
        // GC_GCLAY_PATHNAME=C:\Users\Jerry.HOPA.000\AppData\Local\GameCenter\GCLay.dll
        // GC_GCLAY64_PATHNAME=C:\Users\Jerry.HOPA.000\AppData\Local\GameCenter\GCLay64.dll

        wchar_t *env = GetEnvironmentStringsW();
        int sz = 0;
        while (env[sz] || env[sz + 1]) { ++sz; }

        QString sEnv = QString::fromUtf16((ushort*)env, sz + 1);
        FreeEnvironmentStringsW(env);

        sEnv.append("GC_PROJECT_ID=61");                        sEnv.append(QChar(0));
        sEnv.append("GC_TYPE_ID=0");                            sEnv.append(QChar(0));
        sEnv.append(QString("GC_USER_NAME=%1 %1").arg(nick));   sEnv.append(QChar(0));
        sEnv.append(QString("GC_USER_NICK=%1 %1").arg(nick));   sEnv.append(QChar(0));
        sEnv.append(QString("GC_PERS_ID=" + accId));            sEnv.append(QChar(0));
        sEnv.append(QString("GC_MRAC_TOKEN=" + mracToken));     sEnv.append(QChar(0));
        sEnv.append(QChar(0));

        QString cmdLine = clientPath_ + appFile + QString(" startbypatcher user:%1 _user:%2 token2:%3")
            .arg(userId)
            .arg(accId)
            .arg(token);

        qDebug() << "Starting" << cmdLine;

        HANDLE hProcess = 0;
        HANDLE hThread = 0;
        STARTUPINFOW si{};
        PROCESS_INFORMATION pi{};
        auto r = CreateProcessW(nullptr, (LPWSTR)cmdLine.utf16(), nullptr, nullptr,
                                FALSE,
                                CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
                                (void*)sEnv.utf16(),
                                (LPWSTR)clientPath_.utf16(),
                                &si, &pi);
        if (r == 0)
        {
            lastError_ = GetLastError();
            qWarning() << "Failed to create process:" << lastError_;
            return { false, hProcess, hThread };
        }
        else
        {
            hProcess = pi.hProcess;
            hThread = pi.hThread;
        }

        qDebug() << "exe OK";

        if (!isSuspended)
        {
            ResumeThread(pi.hThread);
        }

        return { true, hProcess, hThread };
    }

    bool MracLaunchApi::startMracDll(HANDLE hProcess)
    {
        auto hModule = GetModuleHandleW(L"kernel32.dll");
        if (hModule == 0)
        {
            lastError_ = GetLastError();
            qWarning() << "Unable to load kernel32:" << lastError_;
            return false;
        }

        uintptr_t pLoadLibraryW = (uintptr_t)GetProcAddress(hModule, "LoadLibraryW");
        if (pLoadLibraryW == 0)
        {
            lastError_ = GetLastError();
            qWarning() << "Unable to find LoadLibraryW:" << lastError_;
            return false;
        }

        //    uint8_t buffer[1000];
        uint8_t buffer[1000] = { 0x68, 0x00, 0x00, 0x00, 0x00,
                                 0xFF, 0x15, 0x00, 0x00, 0x00, 0x00,
                                 0xC2, 0x04 };

        uint8_t * pBlock = (uint8_t *)VirtualAllocEx(hProcess, 0, sizeof(buffer), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!pBlock)
        {
            lastError_ = GetLastError();
            qWarning() << "Failed to allocate mem page" << lastError_;
            return false;
        }

        *(uint32_t *)(buffer + 1) = uint32_t(pBlock + 20);
        *(uint32_t *)(buffer + 7) = uint32_t(pBlock + 16);
        *(uint32_t *)(buffer + 16) = pLoadLibraryW;
        wcscpy((wchar_t *)(buffer + 20), mracDll);

        DWORD written = 0;
        if (WriteProcessMemory(hProcess, pBlock, buffer, sizeof(buffer), &written) != 0)
        {
            assert(written == sizeof(buffer));

            auto hObject = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pBlock, 0, 0, 0);
            if (hObject)
            {
                CloseHandle(hObject);
                qDebug() << "mrac OK";

                lastError_ = 0;
                return true;
            }
            else
            {
                lastError_ = GetLastError();
                qWarning() << "Failed to create thread:" << lastError_;
            }
        }
        else
        {
            lastError_ = GetLastError();
            qWarning() << "Failed to write mem:" << lastError_;
        }

        return false;
    }

    void MracLaunchApi::deleteLauncher()
    {
        if (mracLauncher_)
        {
            mracLauncher_->destroy(mracLauncher_);
            mracLauncher_ = nullptr;
        }
    }

}
