#ifndef util_h
#define util_h

QString spacedNumber(int number);
QString spacedNumber(unsigned number);
QString spacedNumber(long long number);

QString sanitizeFilename(const QString & str);

std::vector<char> toVector(const QByteArray & array);

unsigned toUInt(const QJsonValue & r, unsigned defaultVal = 0);
QJsonValue QJsonValueFromUInt(unsigned v);

#endif