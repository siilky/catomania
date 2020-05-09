#include "stdafx.h"

#include "pckmanager.h"

#pragma warning(disable : 4714)     // ??? \ function 'QString QString::toLower(void) const &' marked as __forceinline not inlined


PckManager::PckManager(const QString & path)
    : file_(path)
    , gbkCodec_(QTextCodec::codecForName("GBK"))
{
}

PckManager::~PckManager(void)
{
}

//

bool PckManager::load()
{
    fileTable.clear();

    if ( ! file_.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open pck file" << file_.fileName();
        return false;
    }

    qint64 fileSize = file_.size();
    if (fileSize < HeaderSize32)
    {
        qWarning() << "Pck file too small";
        return false;
    }

    file_.seek(fileSize - 4);
    const quint32 options = readInt32(file_.read(4));
    is64_ = (options & 0xFF) == 3;

    file_.seek(fileSize - 8);
    const quint32 entryCount = readInt32(file_.read(4));

    file_.seek(fileSize - HeaderSize());
    const quint32 fileTableOffset = readInt32(file_.read(4)) ^ Key1;
    if (fileTableOffset < HeaderSize())
    {
        qWarning() << "Pck file: too small for file table";
        return false;
    }
     
    file_.seek(fileTableOffset);
    for (quint32 i = 0; i < entryCount; i++)
    {
        //quint32 entrySize = readInt32(file_.read(4)) ^ Key1;
        file_.read(4);
        const quint32 entrySize = readInt32(file_.read(4)) ^ Key2;
        QByteArray buffer = file_.read(entrySize);

        // use zlib decompression if compressed_size < decompressed_size
        QPair<QString, FileTableEntry> ft = readTableEntry(entrySize < EntrySize() ? deflate(buffer, EntrySize()) : buffer);
        fileTable.insert(ft.first, ft.second);
// qDebug() << ft.first;
    }

    return true;
}

QPair<QString, PckManager::FileTableEntry> PckManager::readTableEntry(const QByteArray & buffer)
{
    QDataStream ds(buffer);
    ds.setByteOrder(QDataStream::LittleEndian);

    QByteArray bufferString(260, Qt::Uninitialized);
    ds.readRawData(bufferString.data(), 260); 
    bufferString.truncate(bufferString.indexOf('\0'));
    QString filePath;
    if (gbkCodec_)
    {
        filePath = gbkCodec_->toUnicode(bufferString);
        filePath = filePath.toLower();
    }

    FileTableEntry fte;
    if (is64_)
    {
        ds >> fte.fileData;
        ds >> fte.fileDataOffset;
        ds >> fte.fileDataDecompressedSize;
        ds >> fte.fileDataCompressedSize;
    }
    else
    {
        fte.fileData = 0;
        quint32 v;
        ds >> v;
        fte.fileDataOffset = v;
        ds >> fte.fileDataDecompressedSize;
        ds >> fte.fileDataCompressedSize;
    }
    return qMakePair(filePath, fte);
}

QByteArray PckManager::deflate(QByteArray & buffer, quint32 maxSize) const
{
    QByteArray bLength(4, Qt::Uninitialized);
    bLength[0] = (char)((maxSize & 0xFF000000) >> 24);
    bLength[1] = (char)((maxSize & 0x00FF0000) >> 16);
    bLength[2] = (char)((maxSize & 0x0000FF00) >> 8);
    bLength[3] = (char)((maxSize & 0x000000FF) >> 0);
    buffer.prepend(bLength);
    return qUncompress(buffer);
}

QByteArray PckManager::getFile(const QString & fileName) const
{
    if (!file_.isOpen())
        return QByteArray();

    QHash<QString, FileTableEntry>::const_iterator it = fileTable.find(fileName.toLower());
    if (it != fileTable.end())
    {
        const FileTableEntry & ft = it.value();

        const quint64 fileSize = file_.size();
        if (ft.fileDataOffset > fileSize
            || (ft.fileDataOffset + 
                    (ft.fileDataCompressedSize < ft.fileDataDecompressedSize ? ft.fileDataCompressedSize : ft.fileDataDecompressedSize)
                > fileSize))
        {
            qWarning() << "Pck: wrong file record for" << fileName;
            return QByteArray();
        }

        file_.seek(ft.fileDataOffset);
        QByteArray buffer = file_.read(ft.fileDataCompressedSize);
        if (ft.fileDataCompressedSize < ft.fileDataDecompressedSize)
        {
            buffer = deflate(buffer, ft.fileDataDecompressedSize);
        }
        return buffer;
    }

    return QByteArray();
}

quint32 PckManager::readInt32(const QByteArray & byteArray)
{
    if (byteArray.size() < 4)
    {
        return 0;
    }

    return ((unsigned char)byteArray[0]) << 0
         | ((unsigned char)byteArray[1]) << 8
         | ((unsigned char)byteArray[2]) << 16
         | ((unsigned char)byteArray[3]) << 24;
}
