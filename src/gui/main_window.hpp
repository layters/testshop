#ifndef MAIN_WINDOW_HPP_NEROSHOP
#define MAIN_WINDOW_HPP_NEROSHOP

#include <iostream>

#if defined(NEROSHOP_USE_QT)
#include <QMainWindow>
#elif defined(NEROSHOP_USE_DOKUN_UI)
#include <window.hpp>
#else
// do nothing else
#endif

namespace neroshop {

namespace gui {

#if defined(NEROSHOP_USE_QT)
class MainWindow : public QMainWindow {
    Q_OBJECT
#elif defined(NEROSHOP_USE_DOKUN_UI)
class MainWindow : public dokun::Window {     
#else
class MainWindow { 
#endif

public:
    MainWindow();
    //explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

#if defined(NEROSHOP_USE_QT)
private slots:
    void on_actionExit_triggered();
#endif

private:
};

}

}

#endif // MAINWINDOW_H
