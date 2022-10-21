#if defined(NEROSHOP_USE_QT)
#if defined(NEROSHOP_USE_QT_WIDGETS)
#include <QApplication> // For QWidget based Qt applications (QWidget depends on QApplication), but we will be using QML so no need for this
#endif
#include <QGuiApplication> // requires Qt5::Gui
#include <QQmlApplicationEngine> // requires Qt5::Qml
#include <QQmlContext> // QQmlContext *	QQmlApplicationEngine::rootContext()
#include <QQmlComponent> //#include <QQuickView> // requires Qt5::Quick - we're not using QQuickView for now//#include <QQmlComponent>
#include <QStandardPaths>// requires Qt::Core (since Qt 5.0)
#endif

// neroshop (includes both the core headers and the gui headers)
#include "../neroshop.hpp"
using namespace neroshop;

using WalletProxy = gui::Wallet;
using ScriptProxy = gui::Script;

bool isIOS = false;
bool isAndroid = false;
bool isWindows = false;
bool isMac = false;
bool isLinux = false;
bool isTails = false;
bool isDesktop = false;

int main(int argc, char *argv[]) {
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
    
    QQmlApplicationEngine engine;
    //--------------------------
    ////////////////////////////////////////////////////////
    // load all icon images
    if(!Icon::load_all()) {
        neroshop::print("Failed to load all icons", 1);
    }
    ////////////////////////////////////////////////////////    
    // Configuration file must be loaded right after Qt Application object has been created so that we can get the correct config location
    // open configuration script
    neroshop::open_configuration_file();
    // initialize (sync) database
    std::string network_type = Script::get_string(lua_state, "neroshop.monero.daemon.network_type");
    std::cout << "network_type: " << network_type << std::endl;
    // ...
    ////qRegisterMetaType<std::string>("std::string");
    // import paths
    engine.addImportPath(":/fonts"); // import FontAwesome 1.0
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
    
    engine.rootContext()->setContextProperty("neroshopDataDir", NEROSHOP_DEFAULT_DATABASE_PATH);
    engine.rootContext()->setContextProperty("neroshopConfigDir", NEROSHOP_DEFAULT_CONFIGURATION_PATH);
    engine.rootContext()->setContextProperty("neroshopResourcesDir", QCoreApplication::applicationDirPath() + "/" + NEROSHOP_RESOURCES_FOLDER); // defined in icon.hpp//engine.rootContext()->setContextProperty("neroshopResourcesFolder", NEROSHOP_RESOURCES_FOLDER); // defined in icon.hpp
    // create neroshop directories - datadir and configdir
    QString defaultWalletPath = (isWindows) ? (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/neroshop") : (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop");
    if(!std::filesystem::is_directory(defaultWalletPath.toStdString())) {
        neroshop::print(std::string("Creating directory \"") + defaultWalletPath.toStdString() + "\"", 3);
        if(!std::filesystem::create_directories(defaultWalletPath.toStdString())) {
            throw std::runtime_error("Failed to create neroshop datadir");
            return 1;
        }
    }
    ////qmlRegisterType<neroshop::gui::Wallet/*WalletProxy*/>("neroshop.Wallet", 1, 0, "Wallet"); // Usage: import neroshop.Wallet  ...  Wallet { id: wallet }
    // we can also register an instance of a class instead of the class itself
    gui::Wallet wallet;
    engine.rootContext()->setContextProperty("Wallet", &wallet);//new WalletProxy());//qmlRegisterUncreatableType<WalletProxy>("neroshop.Wallet", 1, 0, "Wallet", "Wallet cannot be instantiated directly.");
    // register script
    engine.rootContext()->setContextProperty("Script", new ScriptProxy());//qmlRegisterType<ScriptProxy>("neroshop.Script", 1, 0, "Script");
    //--------------------------
    // When using qrc.qml to store images, use: QUrl(QStringLiteral("qrc:///main.qml"))
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));//("../main.qml");//("main.qml");//(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    return app.exec(); // starts 'event loop'
    #endif
    return 0;
}
