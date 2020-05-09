

class PckManager
{
public:
    PckManager(const QString & path);
    ~PckManager(void);

    QByteArray getFile(const QString & fileName) const;
    bool load();

private:
    static const unsigned FSIG_1 = 1305093103;
    static const unsigned FSIG_2 = 1453361591;
    // #define ASIG_1 -33685778
    // #define ASIG_2 -267534609
    static const unsigned Key1 = 0xA8937462;
    static const unsigned Key2 = 0xF1A43653;
    static const unsigned HeaderSize32 = 272;
    static const unsigned EntrySize32 = 276;
    static const unsigned HeaderSize64 = 280;
    static const unsigned EntrySize64 = 288;

    struct FileTableEntry
    {
        quint32 fileData;
        quint64 fileDataOffset;
        quint32 fileDataDecompressedSize;
        quint32 fileDataCompressedSize;
    };

    QPair<QString, FileTableEntry> readTableEntry(const QByteArray & buffer);
    QByteArray deflate(QByteArray & buffer, quint32 maxSize) const;
    quint32 readInt32(const QByteArray & byteArray);

    unsigned HeaderSize() const
    {
        return is64_ ? HeaderSize64 : HeaderSize32;
    }
    unsigned EntrySize() const
    {
        return is64_ ? EntrySize64 : EntrySize32;
    }

    mutable QFile                   file_;
    QHash<QString, FileTableEntry>  fileTable;
    QTextCodec                      *gbkCodec_;
    bool                            is64_;
};

