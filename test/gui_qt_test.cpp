#include <iostream>
// neroshop
/*#include "../src/neroshop.hpp"
using namespace neroshop;*/
#include "../src/core/config.hpp"
// Qt
#include "main_window.hpp"
#include <QApplication>
#include <QPushButton>

lua_State * neroshop::lua_state = luaL_newstate(); // lua_state should be initialized by default

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //app.setQuitOnLastWindowClosed(false);

    gui::MainWindow * window = new gui::MainWindow();
    window->show();

    QPushButton button ("Hello world!");
    button.show();
    button.setParent(window);
    //-------------------------
    return app.exec(); // .exec starts QApplication and related GUI, this line starts 'event loop'
}
// see: https://doc.qt.io/qt-5/cmake-get-started.html
