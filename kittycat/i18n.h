#ifndef I18N_H
#define I18N_H

#include <QObject>
#include <QTranslator>
#include <QApplication>
#include <QString>
#include <QList>
#include "persistence2.h"

class I18n : public QObject
{
    Q_OBJECT
public:
    I18n(QApplication * application, JsonValue config);
    ~I18n();

    QStringList getLanguages(int * current = NULL) const;   // current is index in list of currently selected language
    bool        setLanguage(const QString & language);      // "" means to remove translation

private:
    Q_DISABLE_COPY(I18n);

    struct Language
    {
        Language(const QString & nm, const QString & loc, const QString & file)
            : name(nm)
            , locale(loc)
            , filename(file)
        {}

        QString name;       // "English" -- language name translated to its own language
        QString locale;     // "en" -- locale name
        QString filename;   // name of .qm file
    };

    enum
    {
        LangAuto    = 0,
        LangNone    = 1,
    };

    bool setSystemLanguage();
    bool setNoLanguage();
    bool loadLanguage(const Language & lang);
    void installTranslator();
    void removeTranslator();

    JsonValue       config_;
    QApplication   *application_;       // weak reference

    QTranslator     translator_;        // translator loaded to application
    QTranslator     qtTranslator_;
    bool            translatorInstalled_;

    QList<Language> languages_;         // available (loaded) languages

    //  0: "English" (actual is no-translation)
    //     ...
    int             currentLanguage_;
};

#endif