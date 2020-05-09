#pragma once

namespace PwMailRu
{

class MracLaunchApi
{
public:
    MracLaunchApi(QString clientPath);
    ~MracLaunchApi();

    bool load();
    bool launch(unsigned userId,
                int (__cdecl *mracCallback)(void *ctx, wchar_t *launchExeFullPath, wchar_t *parameters, wchar_t *directory, void *hProcess),
                int (__cdecl *mracProgressCallback)(void *ctx, int progress),
                void *cbArg);

    std::tuple<bool, HANDLE, HANDLE> 
        MracLaunchApi::createProcess(unsigned userId,
                                     QString accId,
                                     QString nick,
                                     QString token,
                                     QString mracToken,
                                     bool isSuspended);
    bool startMracDll(HANDLE hProcess);

    unsigned error() const
    {
        return lastError_;
    }

private:
    void deleteLauncher();

    QString     clientPath_;
    QLibrary    mrac_;

    void* (__cdecl *createMracLauncher)(int) = nullptr;

    struct mracLauncher
    {
        void(__cdecl *destroy)(mracLauncher *);
        bool(__cdecl *setOption)(mracLauncher *, int type, uint32_t *res);
        bool(__cdecl *getResult)(mracLauncher *, int type, uint32_t *res);
        bool(__cdecl *launch)(mracLauncher *, const wchar_t* launchExeFullPath, const wchar_t *parameters, const wchar_t *directory, uint32_t *r);

    }
    *mracLauncher_ = nullptr;

    unsigned lastError_ = 0;
};

}
