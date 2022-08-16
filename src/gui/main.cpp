#if defined(NEROSHOP_USE_QT) && defined(NEROSHOP_USE_DOKUN_UI) // if both are defined at the same time
#error "You can ONLY define a single GUI at a time"
#endif

#if defined(NEROSHOP_USE_QT)
#include <QApplication>
#include "main_window.hpp"
#elif defined(NEROSHOP_USE_DOKUN_UI)
#include <dokun_ui.hpp>
//using namespace dokun;
#endif
// neroshop (core)
#include "../neroshop.hpp"
using namespace neroshop;
lua_State * neroshop::lua_state = luaL_newstate();

int main(int argc, char *argv[]) {
    #if defined(NEROSHOP_USE_QT)
    QApplication app(argc, argv);
    //app.setQuitOnLastWindowClosed(false);

    gui::MainWindow * window = new gui::MainWindow();
    window->show();

    return app.exec(); // .exec starts QApplication and related GUI, this line starts 'event loop'
    #endif
    return 0;
}
