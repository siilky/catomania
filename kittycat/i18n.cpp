
#include "stdafx.h"

#include <QDebug>
#include <QEvent>
#include <QLocale>
#include <QStringList>
#include <QDir>

#include "i18n.h"

class I18Init
{
public:
    I18Init()
    {
        // Fix a weird issues when Qt looks at the LANG variable and gets confused
        // if system locale differs from it (so our custom fonts disappears in locales
        // where font misses locale glyphs)
        // QLocale::setDefault(locale);  and this doesn't help
        qputenv("LANG", "");    
    }
};

static const I18Init init;      // should be initialized at the very beginning

static const char *i18n_Automatic = QT_TRANSLATE_NOOP("I18n", "Automatic");
//: should be translated to language ID
static const char *i18n_English = QT_TRANSLATE_NOOP("I18n", "English");


I18n::I18n(QApplication * application, JsonValue config)
    : QObject(application)
    , config_(config)
    , application_(application)
    , translatorInstalled_(false)
    , currentLanguage_(LangNone)
{
    Q_ASSERT(application != NULL);

    // 0: "Automatic"
    // 1: "English" (actual is no-translation)
    languages_ << Language(tr(i18n_Automatic), "", "");   // "Automatic" in current locale
    languages_ << Language(tr(i18n_English), "en", "");

    // load translations, get an internationalized string and fill array of locales
    QString langPath = application_->applicationDirPath();
    QDir dir(langPath);
    QStringList files = dir.entryList(QStringList() << "*.qm");
    for (int i = 0; i < files.size(); ++i)
    { 
        QTranslator translator;
        if (translator.load(files[i], langPath))
        {
            QString langName = translator.translate("I18n", "English");
            if ( ! langName.isEmpty())
            {
                languages_ << Language(langName, files[i].section('.', 0, 0).right(2), files[i]);
            }
            else
            {
                qWarning() << "Language name not found in" << files[i];
            }
        }
        else
        {
            qWarning() << "Failed to load translation file" << files[i];
        }
    }

    std::wstring wlang;
    config_.get(L"Selected", wlang);
    if ( ! wlang.empty())
    {
        setLanguage(QString::fromStdWString(wlang));
    }
    else
    {
        // set RU by default
        if ( ! loadLanguage(Language("", "ru", "")))
        {
            setSystemLanguage();
        }
    }

    // update and retranslate
    installTranslator();
}

I18n::~I18n()
{
    removeTranslator();
}

bool I18n::setSystemLanguage()
{
    bool result = false;

    QString locale = QLocale::system().name().left(2);
    result = loadLanguage(Language("", locale, ""));
    if (result)
    {
        currentLanguage_ = LangAuto;
        config_.set(L"Selected", QString(i18n_Automatic).toStdWString());    // 'system' is auto-language
    
        QLocale::setDefault(QLocale::system());
    }
    else
    {
        qWarning() << "Failed to set system language for locale " << locale;
    }

    return result;
}

bool I18n::setNoLanguage()
{
    // set "English" language (actual is no-translation)

    if (currentLanguage_ == LangNone)
    {
        // no change
        return true;
    }

    removeTranslator();
    QLocale::setDefault(QLocale("en"));

    currentLanguage_ = LangNone;
    config_.set(L"Selected", std::wstring());

    return true;
}

QStringList I18n::getLanguages(int * current) const
{
    QStringList result;


    foreach(const Language & lang, languages_)
    {
        result << lang.name;
    }

    if (current != NULL)
    {
        *current = currentLanguage_;
    }

    return result;
}

bool I18n::setLanguage(const QString & language)
{
    bool result = false;

    // "Automatic"
    // "English" (actual is no-translation)
    // ...

    if (language.isEmpty()
        || language == i18n_English)
    {
        result = setNoLanguage();
    }
    else if (language == i18n_Automatic)
    {
        result = setSystemLanguage();
    }
    else
    {
        result = loadLanguage(Language(language, "", ""));
    }

    return result;
}

bool I18n::loadLanguage(const Language & lang)
{
    bool result = false;

    if (languages_.count() <= 2)
    {
        return false;
    }

    for(int i = 2; i < languages_.size(); i++)
    {
        const Language & curLang = languages_[i];
        if (( ! lang.name.isEmpty() && curLang.name == lang.name)
            || ( ! lang.locale.isEmpty() && curLang.locale == lang.locale))
        {
            if (i == currentLanguage_)
            {
                result = true;      // no change
            }
            else
            {
                result = translator_.load(curLang.filename, application_->applicationDirPath(), "", ".qm");
                if (result)
                {
                    qtTranslator_.load(QString("qt_") + curLang.locale, application_->applicationDirPath(), "", ".qm");

                    installTranslator();
                    currentLanguage_ = i;
                    config_.set(L"Selected", curLang.name.toStdWString());

                    QLocale::setDefault(QLocale(curLang.locale));
                }
                else
                {
                    qWarning() << "Failed to load translation for locale" << curLang.locale;
                }
            }
            break;
        }
    }

    return result;
}

void I18n::installTranslator()
{
    if ( ! translatorInstalled_)
    {
        application_->installTranslator(&translator_);
        application_->installTranslator(&qtTranslator_);
        translatorInstalled_ = true;
    }
    else
    {
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(application_, &ev);
    }
}

void I18n::removeTranslator()
{
    if (translatorInstalled_)
    {
        application_->removeTranslator(&translator_);
        application_->removeTranslator(&qtTranslator_);
        translatorInstalled_ = false;
    }
    else
    {
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(application_, &ev);
    }
}
