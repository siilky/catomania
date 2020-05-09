

class SafeFile
{
public:
	SafeFile(const QString & pathname);
	~SafeFile();

    bool read(QByteArray & data);
    bool write(const QByteArray & data);

    void setCaching(bool doCaching)
    {
        doCaching_ = doCaching;
    }
    void setBackup(bool doBackup)
    {
        doBackup_ = doBackup;
    }
    void setChecksum(bool doChecksum)
    {
        doBackup_ = doChecksum;
    }

private:
    bool loadFile(const QString & name, QByteArray & data);
    bool saveFile(const QString & name, const QByteArray & data);

    QString     fileName_;
    bool        doCaching_;
    QByteArray  fileCache_;
    bool        doBackup_;
    bool        doChecksum_;
    bool        keepBackup_;
};
