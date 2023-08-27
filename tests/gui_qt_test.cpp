#include <iostream>
// neroshop
/*#include "../src/neroshop.hpp"
using namespace neroshop;*/
#include "../src/core/settings.hpp"
// Qt
#include "main_window.hpp"
#include <QApplication>
#include <QPushButton>

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
