#if defined(NEROSHOP_USE_QT) && defined(NEROSHOP_USE_DOKUN_UI) // if both are defined at the same time
#error "You can ONLY define a single GUI library at a time"
#endif

#if defined(NEROSHOP_USE_QT)
#if defined(NEROSHOP_USE_QT_WIDGETS)
#include <QApplication> // For QWidget based Qt applications (QWidget depends on QApplication), but we will be using QML so no need for this
#endif
#include <QGuiApplication> // requires Qt5::Gui
#include <QQmlApplicationEngine> // requires Qt5::Qml
#include <QQmlContext> // QQmlContext *	QQmlApplicationEngine::rootContext()
//#include <QQuickView> // requires Qt5::Quick - we're not using QQuickView for now
#include <QStandardPaths>// requires Qt::Core
//#include <QMetaType> // requires Qt::Core
#elif defined(NEROSHOP_USE_DOKUN_UI)
#include <dokun_ui.hpp>
using namespace dokun;
#else
// do nothing else
#endif

// neroshop (includes both the core headers and the gui headers)
#include "../neroshop.hpp"
using namespace neroshop;
lua_State * neroshop::lua_state = luaL_newstate();

int main(int argc, char *argv[]) {
    // load all icon images
    if(!Icon::load_all()) {
        neroshop::print("Failed to load all icons", 1);
    }
    unsigned char * data = Icon::get<unsigned char *>("upload");
    int size = Icon::get<int>("upload");
    // Print icon information
    std::cout << "Icon info (upload): \n" << "data=" << data << "\n" << "size=" << size << "\n";
    ////////////////////////////////////////////////////////
    // create/load configuration script
    if(!neroshop::create_config()) { 
        if(!neroshop::load_config()) {
            neroshop::print("Failed to load config.lua", 1);
        }
    }    
    ////////////////////////////////////////////////////////
    #if defined(NEROSHOP_USE_QT)
    std::cout << "Using Qt version: " << qVersion() << "\n";
    // On Linux desktop, use QApplication since the Qt Labs Platform module uses Qt Widgets as a fallback on platforms that do not have a native platform file dialog implementation available. See https://doc.qt.io/qt-5/qml-qt-labs-platform-filedialog.html#availability
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_QT_WIDGETS)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    #else
    QGuiApplication app(argc, argv);
    #endif
    
    QQmlApplicationEngine engine;
    //--------------------------
    ////qRegisterMetaType<std::string>("std::string");
    // custom macros
    engine.rootContext()->setContextProperty("neroshopAppDirPath", QCoreApplication::applicationDirPath());
    engine.rootContext()->setContextProperty("neroshopVersion", NEROSHOP_VERSION);
    engine.rootContext()->setContextProperty("neroshopDataDir", QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop");//QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    engine.rootContext()->setContextProperty("neroshopConfigDir", QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    engine.rootContext()->setContextProperty("neroshopResourceDir", QCoreApplication::applicationDirPath() + "/" + NEROSHOP_RESOURCES_FOLDER); // defined in icon.hpp
    // create neroshop directories - datadir and configdir
    if(!std::filesystem::is_directory( (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop").toStdString() )) {
        neroshop::print(std::string("Creating directory \"") + (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop").toStdString() + "\"", 3);
        if(!std::filesystem::create_directories( (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop").toStdString() )) {
            throw std::runtime_error("Failed to create neroshop datadir");
            return 1;
        }
    }
    // register custom (class) modules
    ////qmlRegisterType<neroshop::gui::Wallet/*WalletProxy*/>("neroshop.Wallet", 1, 0, "Wallet"); // Usage: import neroshop.Wallet  ...  Wallet { id: wallet }
    // we can also register an instance of a class instead of the class itself
    //gui::Wallet wallet;
    engine.rootContext()->setContextProperty("Wallet", new gui::Wallet()/*WalletProxy()*/);
    //--------------------------
    engine.load("../main.qml");//("main.qml");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    return app.exec(); // starts 'event loop'
    #endif
    ////////////////////////////////////////////////////////
    #if defined(NEROSHOP_USE_DOKUN_UI)
    std::cout << "Using dokun-ui\n";
    #ifdef DOKUN_USE_GLFW
    std::cout << "Using GLFW\n";
    #endif
    // First, initialize engine
    if(!dokun::Engine::open()) {
        std::cout << DOKUN_UI_TAG "engine failed to initialize" << std::endl;
        return 1;
    }    
    // Create window
    gui::MainWindow * window = new gui::MainWindow();
    window->create("neroshop", 1280, 720, 0);
    //window->set_vertical_synchronization(true);
    window->show();
    
    while(window->is_open()) {
        window->set_viewport(window->get_client_width(), window->get_client_height()); // causes "X connection to ? broken (explicit kill or server shutdown)." error
        window->clear(32, 32, 32);	        
        
        window->poll_events();
        
        window->update();
    }
    window->destroy();
    Engine::close(); // will call glfwTerminate();
    #endif
    ////////////////////////////////////////////////////////
    return 0;
}
