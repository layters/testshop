#if defined(NEROSHOP_USE_QT)
#if defined(NEROSHOP_USE_QT_WIDGETS)
#include <QApplication> // For QWidget based Qt applications (QWidget depends on QApplication), but we will be using QML so no need for this
#endif
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> // QQmlContext *	QQmlApplicationEngine::rootContext()
////#include <QQuickView> // we're not using QQuickView for now//#include <QQmlComponent>
#include <QStandardPaths>
#endif

#include "currency_rate_provider.hpp"
#include "backend.hpp"
#include "daemon_manager.hpp"
#include "proxy_manager.hpp"
#include "wallet_manager.hpp"
#include "settings_manager.hpp"
#include "user_manager.hpp"
#include "enum_wrapper.hpp"
#include "wallet_node_provider.hpp"
#include "wallet_qr_provider.hpp"
#include "image_provider.hpp"
#include "notification_manager.hpp"
#include "../neroshop_config.hpp"
#include "../core/tools/filesystem.hpp"
#include "../core/tools/logger.hpp"
#include "../core/version.hpp"
#include "../core/settings.hpp"

using namespace neroshop;

static const QString WALLET_QR_PROVIDER {"wallet_qr"};
static const QString AVATAR_IMAGE_PROVIDER {"avatar"};
static const QString CATALOG_IMAGE_PROVIDER {"listing"};

bool isIOS = false;
bool isAndroid = false;
bool isWindows = false;
bool isMac = false;
bool isLinux = false;
bool isTails = false;
bool isDesktop = false;

int main(int argc, char *argv[])
{
#if defined(NEROSHOP_USE_QT)
    std::cout << "Using Qt version: " << qVersion() << "\n";
    // platform dependant settings
    #if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    bool isDesktop = true;
    #elif defined(Q_OS_ANDROID)
    bool isAndroid = true;
    #elif defined(Q_OS_IOS)
    bool isIOS = true;
    #endif
    #ifdef Q_OS_WIN
    bool isWindows = true;
    #elif defined(Q_OS_LINUX)
    bool isLinux = true;
    #elif defined(Q_OS_MAC)
    bool isMac = true;
    #endif    
    // On Linux desktop, use QApplication since the Qt Labs Platform module uses Qt Widgets as a fallback on platforms that do not have a native platform file dialog implementation available. See https://doc.qt.io/qt-5/qml-qt-labs-platform-filedialog.html#availability
    #if defined(NEROSHOP_USE_QT_WIDGETS)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    #else
    QGuiApplication app(argc, argv);
    #endif
    // Set application name
    app.setApplicationName("neroshop");

    qmlRegisterSingletonType<CurrencyExchangeRatesProvider>(
        "neroshop", 1, 0, "CurrencyExchangeRatesProvider",
        &CurrencyExchangeRatesProvider::qmlInstance);
    qmlRegisterSingletonType(QUrl("qrc:/qml/components/CurrencyExchangeRates.qml"),
        "neroshop.CurrencyExchangeRates", 1, 0, "CurrencyExchangeRates");

    QQmlApplicationEngine engine;
    //--------------------------
    // Configuration file must be loaded right after Qt Application object has been created so that we can get the correct config location
    // open configuration script
    neroshop::load_nodes_from_memory();
    
    // create "datastore" folder within "~/.config/neroshop/" path
    std::string data_dir = neroshop::get_default_database_path();
    if(!neroshop::filesystem::is_directory(data_dir)) {
        neroshop::log_info(std::string("Creating directory \"") + data_dir + "\"");
        if(!neroshop::filesystem::make_directory(data_dir)) {
            throw std::runtime_error("Failed to create neroshop data dir");
            return 1;
        }
    }
    // create "keys" folder within "~/.config/neroshop/" path
    std::string keys_dir = neroshop::get_default_keys_path();
    if(!neroshop::filesystem::is_directory(keys_dir)) {
        neroshop::log_info(std::string("Creating directory \"") + keys_dir + "\"");
        if(!neroshop::filesystem::make_directory(keys_dir)) {
            throw std::runtime_error("Failed to create neroshop keys dir");
            return 1;
        }
    }
    // start database
    Backend::initializeDatabase();
    // import paths
    engine.addImportPath(":/assets/fonts"); // import FontAwesome 1.0
    // platform macros
    engine.rootContext()->setContextProperty("isLinux", isLinux);
    engine.rootContext()->setContextProperty("isWindows", isWindows);
    engine.rootContext()->setContextProperty("isMac", isMac);
    engine.rootContext()->setContextProperty("isDesktop", isDesktop);
    engine.rootContext()->setContextProperty("isAndroid", isAndroid);
    engine.rootContext()->setContextProperty("isIOS", isIOS);
    ////engine.rootContext()->setContextProperty("isTails", isTails);
    // custom macros
    engine.rootContext()->setContextProperty("neroshopAppDirPath", QCoreApplication::applicationDirPath());
    engine.rootContext()->setContextProperty("neroshopVersion", NEROSHOP_VERSION);
    engine.rootContext()->setContextProperty("neroshopDataDirPath", QString::fromStdString(neroshop::get_default_config_path()));
    engine.rootContext()->setContextProperty("neroshopDefaultWalletDirPath", QString::fromStdString(neroshop::get_default_wallet_path()));
    // create neroshop wallet directory
    if(!neroshop::filesystem::is_directory(neroshop::get_default_wallet_path())) {
        neroshop::log_info(std::string("Creating directory \"") + neroshop::get_default_wallet_path() + "\"");
        if(!neroshop::filesystem::make_directory(neroshop::get_default_wallet_path())) {
            throw std::runtime_error("Failed to create neroshop wallet dir");
            return 1;
        }
    }    
    // Create an instance of DaemonManager and expose it to QML
    DaemonManager * daemonManager = new DaemonManager(&engine);
    daemonManager->startDaemonProcessDetached();
    engine.rootContext()->setContextProperty("DaemonManager", daemonManager);
    ProxyManager * proxyManager = new ProxyManager(&engine);
    engine.rootContext()->setContextProperty("ProxyManager", proxyManager);
    engine.setNetworkAccessManagerFactory(proxyManager);
    // Register WalletManager (wallet)
    WalletManager *wallet = new WalletManager(&engine);
    auto walletType = static_cast<neroshop::WalletType>(wallet->getWalletType());
    engine.rootContext()->setContextProperty("Wallet", wallet);//qmlRegisterUncreatableType<WalletProxy>("neroshop.WalletManager", 1, 0, "Wallet", "Wallet cannot be instantiated directly.");//qmlRegisterType<WalletManager>("neroshop.Wallet", 1, 0, "Wallet"); // Usage: import neroshop.Wallet  ...  Wallet { id: wallet }
    qRegisterMetaType<WalletManager*>(); // Wallet can now be used as an argument in function parameters
    // Register WalletNodeProvider
    WalletNodeProvider* walletNodeProvider = WalletNodeProvider::instance();//QQmlEngine::setObjectOwnership(walletNodeProvider, QQmlEngine::CppOwnership);
    QString coinName;
    switch(walletType) {
        case neroshop::WalletType::Monero:
            coinName = "monero";
            break;
        case neroshop::WalletType::Wownero:
            coinName = "wownero";
            break;
        default:
            throw std::runtime_error("Invalid wallet type");
    }
    walletNodeProvider->setCoinName(coinName);
    walletNodeProvider->startUpdates();
    engine.rootContext()->setContextProperty("WalletNodeProvider", walletNodeProvider);//qmlRegisterSingletonInstance("neroshop", 1, 0, "WalletNodeProvider", WalletNodeProvider::instance());
    // Register settings - used in both QML and C++, so had to make it a singleton
    SettingsManager& settingsManager = SettingsManager::instance();//qDebug() << "QSettings file path:" << settingsManager.fileName();
    settingsManager.initializeDefaults();
    engine.rootContext()->setContextProperty("Settings", &settingsManager);
    // Register backend
    engine.rootContext()->setContextProperty("Backend", new Backend(&engine));
    // Register user
    engine.rootContext()->setContextProperty("User", new UserManager(&engine));
    qRegisterMetaType<UserManager *>();
    // Register all enums within the "neroshop" namespace
    ////qmlRegisterUncreatableMetaObject(neroshop::staticMetaObject, "neroshop.namespace", 1, 0, "Neroshop", "Error: only enums");
    // TableModel
    ////qmlRegisterType<TableModel>("neroshop.TableModel", 1, 0, "TableModel"); // Usage: import neroshop.TableModel  ...  TableModel { id: tableModel }
    // Register enums
    qmlRegisterUncreatableType<EnumWrapper>("neroshop.Enums", 1, 0, "Enum", "Enums cannot be created.");//neroshop::EnumWrapper::registerEnums(&engine);

    engine.addImageProvider(WALLET_QR_PROVIDER, new WalletQrProvider(WALLET_QR_PROVIDER));
    engine.addImageProvider(AVATAR_IMAGE_PROVIDER, new ImageProvider(AVATAR_IMAGE_PROVIDER));
    engine.addImageProvider(CATALOG_IMAGE_PROVIDER, new ImageProvider(CATALOG_IMAGE_PROVIDER));    
    //--------------------------
    // Load main.qml from the "qml/" directory
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    
    QObject *rootObject = engine.rootObjects().first();
    QObject *toastObject = rootObject->findChild<QObject*>("toast");
    if (!toastObject) {
        qWarning() << "Toast object not found!";
        return -1;
    }
    // Create NotificationManager with toastObject pointer
    NotificationManager::instance(toastObject);
    // Usage: NotificationManager::instance()->showToast("Message");
    
    return app.exec(); // starts 'event loop'
    #endif
    return 0;
}
