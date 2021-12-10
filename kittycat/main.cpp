
#include "stdafx.h"

#include "main.h"
#include "clientlocation.h"
#include "config.h"
#include "forms/uiclientlocation.h"
#include "forms/uimultiview.h"
#include "game/data/elements.h"
#include "game/data/gshop.h"
#include "i18n.h"
#include "license.h"
#include "log.h"
#include "minidump.h"
#include "persistence2.h"
#include "qlib/game/iconmanager.h"
#include "qlib/pckmanager.h"
#include "version.h"

// test/debug
//#include "marketbuilder.h"


elements::ItemListCollection  g_elements_;
QString g_elementExePath_;
bool g_isSafeProcessMode = false;

static ErrorState loadElements(const wchar_t *elements, const wchar_t *config)
{
    elements::ItemListCollection data;

#ifndef _DEBUG_
    ErrorState error = data.load(elements, config);
    if (error.code != ERR_NO_ERROR && error.code != ERR_PARTIAL_DATA)
    {
        g_elements_ = elements::ItemListCollection();
        return error;
    }
#else
    (void) elements;
    (void) config;
#endif

    g_elements_ =
        data[elements::WEAPONS] + data[elements::ARMORS] + data[elements::ORNAMENTS]
        + data[elements::POTIONS]
        + data[elements::MOLDS]
        + data[elements::ATK_CHARMS] + data[elements::DEF_CHARMS]
        + data[elements::SKILLS]
        + data[elements::FLY_MOUNTS] + data[elements::ELF_WINGS] + data[elements::TELEPORTS]
        + data[elements::KEY_ITEMS]
        + data[elements::CHI_STONES]
        + data[elements::QUEST_ITEMS]
        + data[elements::AMMUNITION] + data[elements::AMMO] + data[elements::QUIVERS]
        + data[elements::SOULGEMS]
        + data[elements::QUEST_ACTIVATORS] + data[elements::QUEST_REWARDS]
        //+ data[elements::RESOURCES]
        + data[elements::FASHION]
        + data[elements::TRANS_SCROLLS] //+ data[elements::HEAD_BOLUS]
        + data[elements::PET_EGGS] + data[elements::PET_FOOD] + data[elements::PET_TRANSFIGURATION]
        + data[elements::FIREWORKS]
        + data[elements::CATAPULT]
        + data[elements::CHEMISTRY]
        + data[elements::REFINING]
        + data[elements::HEAVEN_BOOKS]
        + data[elements::CHAT]
        + data[elements::HIEROGRAMS_HP] + data[elements::HIEROGRAMS_MP]
        + data[elements::EXP_MULTIPLIERS]
        + data[elements::TELEPORT_SCROLLS]
        + data[elements::DYE]
        + data[elements::GENIES] + data[elements::GENIE_ARMOR] + data[elements::GENIE_EXP_CUBES]
        + data[elements::VENDOR_SKINS]
        + data[elements::SPY_EYE]
        + data[elements::CRAFT_SCROLLS]
        + data[elements::RUNES]
        + data[elements::GUILD_MATERIALS]
        + data[elements::CONGREGATE_ESSENCE]
        + data[elements::VANGUARD_ODRERS]
        + data[elements::BANKNOTES]
        + data[elements::UNIVERSAL_TOKEN]
        + data[elements::CARDS]
        + data[elements::ASTROLABE_ESSENCE]
        + data[elements::ASTROLABE_RANDOM_ADDON_ESSENCE]
        + data[elements::ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE]
        + data[elements::ASTROLABE_INC_EXP_ESSENCE]
        + data[elements::ITEM_PACKAGE_BY_PROFESSION_ESSENCE]
        + data[elements::FIREWORKS2_ESSENCE]
        + data[elements::FIX_POSITION_TRANSMIT_ESSENCE]
        + data[elements::HOME_FORMULAS_ITEM_ESSENCE]
        + data[elements::WISH_TRIBUTE_ESSENCE]
        + data[elements::EASY_PRODUCE_ITEM_ESSENCE]
        + data[elements::UNLOCK_RUNE_SLOT_ITEM_ESSENCE]
        + data[elements::RUNE_ITEM_ESSENCE]
        + data[elements::FIX_MONSTER_ITEM_ESSENCE]
        + data[elements::ITEM_USED_FOR_AREA_ESSENCE]
        + data[elements::CAPTURE_ITEM_ESSENCE]
        + data[elements::BIBLE_REFINE_TICKET_ESSENCE]
        + data[elements::NEW_ARMOR_ESSENCE]
        + data[elements::QIHUN_ESSENCE]
        + data[elements::QILING_ESSENCE]
        + data[elements::STORY_BOOK_MAJOR_TYPE]
        + data[elements::STORY_BOOK_CONFIG]
        + data[elements::QIHUN_COVER_ESSENCE]
        + data[elements::RED_BOOK_UPGRADE_ITEM]
        + data[elements::CAMP_TOKEN_ESSENCE]
        + data[elements::FASHION_NEW_ESSENCE]
        + data[elements::ILLUSTRATED_FASHION_ESSENCE]
        + data[elements::ILLUSTRATED_WING_EGG_ESSENCE]
        + data[elements::ILLUSTRATED_PET_EGG_ESSENCE]
        + data[elements::FAST_PRODUCE_ITEM_ESSENCE]
        + data[elements::KID_DEBRIS_ESSENCE]
        + data[elements::KID_DEBRIS_GENERATOR_ESSENCE]
        ;

    return error;
}

static bool isClientUnpacked(const QString & clientPathName)
{
    QFile f(clientPathName);
    if (!f.open(QIODevice::ReadOnly))
    {
        return false;
    }
    quint64 size = f.size();
    if (size == 0
        || size >= std::numeric_limits<unsigned>().max() //as we use unsigneds in array
        )
    {
        return false;
    }

    uchar *p = f.map(0, size);
    if (p == 0)
    {
        return false;
    }

    // get enthropy

    unsigned array[256] = { 0 };
    for (quint64 i = 0; i < size; i++)
    {
        array[p[i]] ++;
    }

    double enthropy = 0;
    for (int i = 0; i < 256; i++)
    {
        double freq = (double) array[i] / size;
        enthropy -= freq * log2(freq);
    }

    return enthropy < 7;    // 6.5 on unpacked, 7.9 packed
}

static bool queryUserClientLocation(QString & clientPath, QString & exeName)
{
    ClientLocationView clv;
    clv.setPath(clientPath);
    clv.setExeName(exeName);

    int r;
    if ((r = clv.exec()) == QDialog::Accepted)
    {
        clientPath = clv.getPath();
        exeName = clv.getExeName();
        return true;
    }
    return false;
}

static bool loadClientFiles(Persistence & config)
{
    QString elementConfigPath = QApplication::applicationDirPath() + QDir::separator() + "elements.cfg";
    if ( ! QFile(elementConfigPath).exists())
    {
        QMessageBox::critical( 0
                             , QApplication::applicationName()
                             , QObject::tr("Could not find elements configuration file.\n"
                                           "Please check your installation."));
        return false;
    }

    std::wstring cd, exe;
    config.root().get(L"ClientDir", cd);
    config.root().get(L"ExeName", exe);

    QString clientDir = QString::fromStdWString(cd);
    QString exeName = QString::fromStdWString(exe);

    if (clientDir.isEmpty())
    {
        clientDir = getClientLocation();
    }

    bool loadFailed = false;
    do 
    {
        if ((clientDir.isEmpty() || loadFailed)
            && !queryUserClientLocation(clientDir, exeName))
        {
            return false;
        }

        {
            QDir dir(clientDir);

            bool hasFiles = dir.exists() 
                && dir.cd("element")
                && dir.exists("surfaces.pck")
                && dir.cd("data")
                && dir.exists("elements.data")
                && dir.exists("gshop.data");

            if (!hasFiles)
            {
                QMessageBox::critical( 0
                                     , QApplication::applicationName()
                                     , QObject::tr("Path %1 does not seem to contain required files.\n"
                                                   "Please choose another game path.")
                                        .arg(dir.absolutePath()));
                loadFailed = true;
                continue;
            }
        }

        QDir dir(clientDir);
        dir.cd("element");

        g_elementExePath_ = dir.filePath(exeName.isEmpty() ? "elementclient.exe" : exeName);

        if (!dir.exists(g_elementExePath_))
        {
            QMessageBox::critical(0
                                  , QApplication::applicationName()
                                  , QObject::tr("File %1 is missing.\n"
                                                "Please choose another game path or use different executable option.")
                                  .arg(g_elementExePath_));
            loadFailed = true;
            continue;
        }


    #if defined(GAME_USING_CLIENT)
        if (!isClientUnpacked(g_elementExePath_))
        {
            QMessageBox::critical(0
                                  , QApplication::applicationName()
                                  , QObject::tr("File %1 seems to be packed and not usable.\n"
                                                "Please choose another game path or use different executable option.")
                                  .arg(g_elementExePath_));
            loadFailed = true;
            continue;
        }
    #endif

        dir.cd("data");

        {
            QString elementPath = dir.filePath("elements.data");
            auto result = loadElements(elementPath.toStdWString().c_str(), elementConfigPath.toStdWString().c_str());
            if (result.code == ERR_PARTIAL_DATA)
            {
                QMessageBox::warning(0
                                      , QApplication::applicationName()
                                      , QObject::tr("Elements file %1 was loaded partially due to config mismatch\n"
                                                    "which may affect using of item properties.\n"
                                                    "Make sure you have latest version of application.")
                                      .arg(elementPath));
            }
            else if (result.code != ERR_NO_ERROR)
            {
                QMessageBox::critical( 0
                                     , QApplication::applicationName()
                                     , QObject::tr("Elements file %1 failed to load.\n"
                                                   "Please choose another game path.")
                                        .arg(elementPath));
                continue;
            }
        }

        {
            QString gshopPath = dir.filePath("gshop.data");
            loadFailed = !GShop::GShop::instance()->load(gshopPath.toStdWString().c_str());
            if (loadFailed)
            {
                QMessageBox::critical( 0
                                     , QApplication::applicationName()
                                     , QObject::tr("GShop file %1 failed to load.\n"
                                                   "Please choose another game path.")
                                        .arg(gshopPath));
                continue;
            }
        }

        dir.cd("..");

        {
            QString sPath = dir.filePath("surfaces.pck");
            PckManager pckManager(sPath);

            loadFailed = !pckManager.load();
            if (loadFailed)
            {
                QMessageBox::critical(0
                                      , QApplication::applicationName()
                                      , QObject::tr("Surfaces file %1 failed to load.\n"
                                                    "Please choose another game path.")
                                        .arg(sPath));
                continue;
            }

            QByteArray items(pckManager.getFile("surfaces\\iconset\\iconlist_ivtrm.dds"));
            QByteArray desc(pckManager.getFile("surfaces\\iconset\\iconlist_ivtrm.txt"));
            IconManager::instance()->load(desc, items);
        }
    } while (loadFailed);

    config.root().set(L"ClientDir", clientDir.toStdWString());
#if defined(GAME_USING_CLIENT)
    config.root().set(L"ExeName", exeName.toStdWString());
#endif

    return true;
}

int main(int argc, char *argv[])
{
//     _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_EVERY_1024_DF);
#ifndef _DEBUG
    SetUnhandledExceptionFilter(crashHandler);
    qlib::exceptionHandler = crashHandler;
#endif

    qDebug() << "Start";    // initialize logging to debugger
    debugInit(L"cats2");

    QTime time = QTime::currentTime();
    qsrand(time.minute() * time.msec());

    QApplication app(argc, argv);
    app.setApplicationVersion(QString::fromWCharArray(g_revision));
    app.setApplicationName("Cat");
    app.setWindowIcon(QIcon(":/catView/c48.png"));

//testBuildMarketList();

#if defined(GAME_USING_CLIENT)
    QCommandLineParser parser;
    parser.addOptions(
        {
            {{"s", "safe"},
            QCoreApplication::translate("main", "Safe client mode.") },
        }
    );
    parser.process(app);
    g_isSafeProcessMode = parser.isSet("safe");
#endif

    QString configLocation = app.applicationDirPath() + QDir::separator() + "cats2.jcfg";
    bool restored;
    Persistence config;
    if ( ! config.open(configLocation.toStdWString(), restored))
    {
        if ( ! QFile::exists(configLocation))
        {
            // create new file and retry
            QFile newConfig(configLocation);
            if ( ! newConfig.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                QMessageBox::critical( 0
                                     , QApplication::applicationName()
                                     , QString("Failed to create config file %1").arg(configLocation));
                return 1;
            }
            newConfig.write("{}");
            newConfig.close();

            if ( ! config.open(configLocation.toStdWString(), restored))
            {
                QMessageBox::critical( 0
                                     , QApplication::applicationName()
                                     , QString("Failed to load new config file %1").arg(configLocation));
                return 1;
            }
        }
        else
        {
            // file exists and cannot be loaded in any ways
            QMessageBox::critical( 0
                                 , QApplication::applicationName()
                                 , QString("Failed to load config file %1").arg(configLocation));
            return 1;
        }
    }

    // Config loaded, more or less
    
    I18n translator(&app, config.root().get(L"Language"));

    if (restored)
    {
        QMessageBox::information( 0
                                , QApplication::applicationName()
                                , QObject::tr("Config file %1 failed to load and was restored from previously saved state. "
                                    "Please check your market setup and prices are in actual state for all cats before opening."
                                    ).arg(configLocation));
    }

    if ( ! validateLicense())
    {
        return 2;
    }

    if (g_isSafeProcessMode)
    {
        std::wstring cd, exe;
        config.root().get(L"ClientDir", cd);
        config.root().get(L"ExeName", exe);

        QString clientDir = QString::fromStdWString(cd);
        QString exeName = QString::fromStdWString(exe);

        QDir dir(clientDir);
        dir.cd("element");
        g_elementExePath_ = dir.filePath(exeName.isEmpty() ? "elementclient.exe" : exeName);
    }
    else if (!loadClientFiles(config))
    {
        return 3;
    }
    
    qApp->setProperty("config", QVariant::fromValue((void*)&config));

    int result;
    {   // scope guard
        MultiView mw(config.root(), &translator);
        mw.show();

        result = app.exec();
    }

    return result;
}
