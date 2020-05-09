#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QThread>

#ifndef Q_MOC_RUN
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winnt.h>
#endif

class DebuggerPrivate;

class Debugger : public QThread  // thread-owned object
{
    Q_OBJECT
    Q_DISABLE_COPY(Debugger)
public:
    Debugger(QObject *parent = 0);
    virtual ~Debugger();

    bool startProcess(const QString & executable, const QString & currentDir = QString());

    bool addBreakpoint(quintptr addr);
    bool removeBreakpoint(quintptr addr);
    bool continueDebugging(unsigned threadId);

    void stopDebugging();

    //QString errorString() const;
    unsigned lastError() const;

    unsigned processId() const;

    bool getContext(unsigned threadId, CONTEXT *ctx);
    bool setContext(unsigned threadId, CONTEXT* ctx);

    QByteArray readMemory(quintptr addr, unsigned size) const;
    bool writeMemory(quintptr addr, const char *data, unsigned size) const;

    void flushMemory();

// no slots because thread runs without event loop and cannot handle slots

signals:
    void debuggingStarted();
    void debuggingStopped();

    void breakpoint(quintptr addr, unsigned threadId);

protected:
    virtual void run();

    //Debugger(DebuggerPrivate &&d, QObject *parent);

    DebuggerPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE(Debugger)
    friend class Breakpoint;

    void stopDebugger(bool terminateProcess = true);
    void setMemPriority();
};

#endif
