
#include "stdafx.h"

#include "util.h"


static QString insertSpaces(QString s);

QString spacedNumber(int number)
{
    if (number < 0)
    {
        QString r = insertSpaces(QString::number(qAbs(number)));
        r.prepend('-');
        return r;
    }

    return insertSpaces(QString::number(number));
}

QString spacedNumber(unsigned number)
{
    return spacedNumber(long long(number));
}

QString spacedNumber(long long number)
{
    if (number < 0)
    {
        QString r = insertSpaces(QString::number(qAbs(number)));
        r.prepend('-');
        return r;
    }

    return insertSpaces(QString::number(number));
}

//

static QString insertSpaces(QString s)
{
    int i = s.count() - 3;
    while (i > 0)
    {
        s.insert(i, ' ');
        i -= 3;
    }

    return s;
}

QString sanitizeFilename(const QString & str)
{
    // cleanup filename
    QString result(str);
    result.replace('\\', "%5C");
    result.replace('/', "%2F");
    result.replace(':', "%3A");
    result.replace('*', "%2A");
    result.replace('?', "%3F");
    result.replace('\"', "%22");
    result.replace('>"', "%3E");
    result.replace('<"', "%3C");
    result.replace('|"', "%7C");
    return result;
}

std::vector<char> toVector(const QByteArray & array)
{
    std::vector<char> r;
    std::copy(array.constBegin(), array.constEnd(), std::back_inserter(r));
    return r;
}

unsigned toUInt(const QJsonValue & r, unsigned defaultValue /*= 0*/)
{
    // потому что числа читаются как дабл, бгбг

    if (r.type() == QJsonValue::Double)
    {
        double d = r.toDouble();
        unsigned v = unsigned(d);
        assert(v == d);
        if (v == d)
        {
            return v;
        }
    }
    return defaultValue;
}

QJsonValue QJsonValueFromUInt(unsigned v)
{
    return QJsonValue(double(v));
}
