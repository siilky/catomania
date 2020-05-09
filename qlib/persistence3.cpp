
#include "stdafx.h"

#include "persistence3.h"


static bool saveFile(const QString & filename, const QByteArray & data)
{
    assert( ! filename.isEmpty());

    bool success = false;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        quint64 written = file.write(data);
        if (written == data.size())
        {
            success = true;
        }
        else
        {
            qWarning() << "Failed to write new config, not all data written:" << written << "of" << data.size();
            assert(0);
        }

        file.close();
    }

    return success;
}

static bool loadFile(const QString & filename, QByteArray & data)
{
    assert( ! filename.isEmpty());

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open config file" << filename;
        return false;
    }

    data = file.readAll();
    return true;
}

//

bool JsonValue::saveTo(const QString & filename)
{
    QJsonDocument json;
    json.setObject(*this);
    QByteArray jsonText = json.toJson();
    return saveFile(filename, jsonText);
}

bool JsonValue::loadFrom(const QString & filename)
{
    QByteArray jsonText;
    if (loadFile(filename, jsonText))
    {
        QJsonDocument json = QJsonDocument::fromJson(jsonText);
        if (!json.isNull() && json.isObject())
        {
            QJsonObject::operator =(json.object());
            return true;
        }
    }
    return false;
}
Persistence::~Persistence()
{
    save();
}

bool Persistence::open(const QString & filename, bool & restored)
{
    isRestored_ = restored = false;

    if (fileName_ == filename)
    {
        // already opened
        return true;
    }

    fileCache_.clear();
    bool success = loadFile(filename, fileCache_);
    if (success)
    {
        json_ = QJsonDocument::fromJson(fileCache_);
        success = ! json_.isNull();
    }

    // if failed, try loading 'new' config
    if ( ! success)
    {
        success = loadFile(filename + ".n", fileCache_);
        if (success)
        {
            json_ = QJsonDocument::fromJson(fileCache_);
            success = !json_.isNull();

            if (success)
            {
                isRestored_ = true;
            }
        }
    }

    // if still failed try to load old file
    if (!success)
    {
        success = loadFile(filename + ".o", fileCache_);
        if (success)
        {
            json_ = QJsonDocument::fromJson(fileCache_);
            success = !json_.isNull();

            if (success)
            {
                isRestored_ = true;
            }
        }
    }

    if (success)
    {
        fileName_ = filename;   // means loaded
    }
    else
    {
        fileCache_.clear();
    }

    restored = isRestored_;
    return success;
}

bool Persistence::save()
{
    if (fileName_.isEmpty())
    {
        // not opened
        return true;
    }

    QByteArray jsonText = json_.toJson();

    if ( ! isRestored_
        && jsonText == fileCache_)
    {
        // при совпадении не нужно сохранять, если мы не начинали с восстановления
        return true;
    }

    if (!saveFile(fileName_ + ".n", jsonText))
    {
        return false;
    }

    fileCache_ = jsonText;
    isRestored_ = false;

    // remove old file if exists
    QFile oldFile(fileName_ + ".o");
    if (oldFile.exists())
    {
        oldFile.remove();
    }

    // move current to old
    QFile curFile(fileName_);
    if (!curFile.rename(fileName_ + ".o"))
    {
        // may not exist
        qDebug() << "Move previous config failed";
        // try removing if cant rename
        if (curFile.exists())
        {
            curFile.remove();
        }
    }

    // move new to current
    QFile newFile(fileName_ + ".n");
    if (!newFile.rename(fileName_))
    {
        qDebug() << "Move new config failed";
        assert(0);
    }

    return true;
}

//

// void JsonValue::save()
// {
//     if (persistence_ != NULL)
//     {
//         persistence_->save();
//     }
// }


// std::wstring JsonValue::string() const
// {
//     return json::write(*this);
// }
// 
// bool JsonValue::read(const std::wstring & string)
// {
//     return json::read(string, *this);
// }
