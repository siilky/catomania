#include <stdafx.h>

#include "safefile.hpp"


static const QString oldSuffix = ".o";
static const QString currentSuffix = "";
// static const QString newSuffix = ".n";

SafeFile::SafeFile(const QString & pathname)
    : fileName_(pathname)
    , doCaching_(true)
    , doBackup_(true)
    , doChecksum_(true)
    , keepBackup_(false)
{
}

SafeFile::~SafeFile()
{
}

bool SafeFile::read(QByteArray & data)
{
    if (doCaching_ && !fileCache_.isNull())
    {
        data = fileCache_;
        return true;
    }

    keepBackup_ = false;
    bool loaded = false;

    QString oldName(fileName_ + oldSuffix);
    QString curName(fileName_ + currentSuffix);

    // loading of nonexistent is ok
    if (!QFile::exists(oldName)
        && !QFile::exists(curName))
    {
        data = QByteArray();
        return true;
    }

    // try loading usual file
    loaded |= loadFile(curName, data);
    // when failed try to load old backup
    if (doBackup_ && !loaded)
    {
        loaded = loadFile(oldName, data);
        if (loaded)
        {
            keepBackup_ = true;
        }
    }
    if (doCaching_ && loaded)
    {
        fileCache_ = data;
    }

    return loaded;
}

bool SafeFile::write(const QByteArray & data)
{
    QString oldName(fileName_ + oldSuffix);
    QString curName(fileName_ + currentSuffix);

    if (doBackup_ && !keepBackup_)
    {
        if (QFile::exists(curName))
        {
            // that means filename (most probably!) was correctly loaded
            if (QFile::exists(oldName)
                && !QFile::remove(oldName))
            {
                qWarning() << "Failed to remove" << oldName;
                return false;
            }

            if (!QFile::rename(curName, oldName))
            {
                qWarning() << "Failed to rename" << curName << "to" << oldName;
                return false;
            }
        }
    }

    bool saved = saveFile(curName, data);

    if (doBackup_ && !saved)
    {
        // we failed to save new file so need to keep old backup
        keepBackup_ = true;
    }
    if (doCaching_ && saved)
    {
        fileCache_ = data;
    }

    return saved;
}

//

bool SafeFile::loadFile(const QString & name, QByteArray & data)
{
    QFile file(name);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    data = file.readAll();
    if (doChecksum_)
    {
        if (data.size() < 2)
        {
            // too small
            return false;
        }

        QByteArray chk = data.right(2);
        quint16 sum = ((quint16(chk.at(0)) & 0xFF) << 8)
            | (quint16(chk.at(1)) & 0xFF);

        data.truncate(data.size() - 2);
        quint16 calculated = qChecksum(data.data(), data.size());

        return calculated == sum;
    }
    return true;
}

bool SafeFile::saveFile(const QString & name, const QByteArray & data)
{
    QByteArray toSave(data);
    if (doChecksum_)
    {
        quint16 calculated = qChecksum(toSave.data(), toSave.size());
        toSave.append(char((calculated >> 8) & 0xFF));
        toSave.append(char(calculated & 0xFF));
    }

    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered))
    {
        qWarning() << "Failed to open file for writing:" << name;
        assert(0);
        return false;
    }

    quint64 written = file.write(toSave);
    if (written != toSave.size())
    {
        qWarning() << "Failed to write file, not all data written:" << written << "of" << toSave.size();
        assert(0);
        return false;
    }

    file.close();
    return true;
}
