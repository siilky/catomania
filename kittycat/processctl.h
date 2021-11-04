#ifndef PROCESSCTL_H
#define PROCESSCTL_H

#include <QObject>

#include "types.h"

class MemoryAccess;
class Debugger;

class ProcessCtl : public QObject
{
    Q_OBJECT
public:
    struct PatchData
    {
        unsigned        hint;
        unsigned        lookupLength;
        const std::vector<unsigned short>   pattern;
        int             offset;
        const barray    patch;
    };

    struct BreakpointData
    {
        unsigned        hint;
        unsigned        lookupLength;
        const std::vector<unsigned short>   pattern;
        int             offset;
    };

    ProcessCtl(QObject *parent = 0);
    ~ProcessCtl();

    void setSafeMode(bool isOn)
    {
        isSafeMode_ = isOn;
    }

    void setServerSubstitution(const QString & address, unsigned port);
    bool start(const QString & executable, const QString & user, const QString & password);
    void stop();

    bool selectChar(const QString & charName);

    void showWindow();
    void hideWindow();
    void flushMemory();

signals:
    void started();
    void stopped();
    void error(const QString & message);
    void windowClosed();    // user has closed window manually

private slots:
    void breakpoint(quintptr addr, unsigned threadId);

    bool replaceRoleStart();

private:
    void init();
    bool applyPatch(const PatchData & patchData);
    quintptr placeBreakpoint(const BreakpointData & breakpoint);

    bool getSettingsPtr(unsigned threadId);
    bool getWindowHandle(unsigned threadId);
    bool processCloseWindow(unsigned threadId);
    bool replaceServer(unsigned threadId);
    bool replaceServer2(unsigned threadId);
    bool replaceServer3(unsigned threadId);
    bool replaceRoleStep();

    Debugger    *debugger_;
    QScopedPointer<MemoryAccess>    memoryAccess_;
    
    bool        isStartup_;

    quintptr    adjustSettingsBp_;
    quintptr    pSettings_;

    quintptr    createWindowBp_;
    HWND        windowHandle_;
    quintptr    closeWindowBp_;

    quintptr    replaceServerBp_;
    quintptr    replaceServerBp2_;
    quintptr    replaceServerBp3_;
    QString     serverAddress_;
    unsigned    serverPort_;

    quintptr    replaceRoleBp_;
    quintptr    replaceRoleEndBp_;
    quintptr    replaceRoleStepBp_;
    unsigned    replaceRoleThreadId_;
    QString     role_;

    bool        isSafeMode_ = false;
};

#endif
