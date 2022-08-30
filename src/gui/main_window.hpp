#pragma once

#ifndef MAIN_WINDOW_HPP_NEROSHOP
#define MAIN_WINDOW_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#if defined(NEROSHOP_USE_QT_WIDGETS)
//#include <QMainWindow> // requires Qt5::Widgets // refer to: https://doc.qt.io/qt-6/qmainwindow.html
#endif
#include <QObject>

#elif defined(NEROSHOP_USE_DOKUN_UI)
#include <window.hpp>
#else
// do nothing else
#endif

#include <iostream>

namespace neroshop {

namespace gui {

//#if defined(NEROSHOP_USE_QT) && defined(NEROSHOP_USE_QT_WIDGETS)
//class MainWindow : public QMainWindow, public QObject {
//    Q_OBJECT 
#if defined(NEROSHOP_USE_QT)
class MainWindow : public QObject {
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
    // properties
    ////Q_PROPERTY(std::string name READ ...)
    // functions
    // this function can be called within QML
    Q_INVOKABLE QString get_sumtin() const {
        return "I am getting sumtin";
    }  
    Q_INVOKABLE void say_sumtin() const {
        std::cout << "I am saying sumtin" << std::endl;
    }

#if defined(NEROSHOP_USE_QT)
signals:
private slots:
#if defined(NEROSHOP_USE_QT) && defined(NEROSHOP_USE_QT_WIDGETS)
    void on_actionExit_triggered();
#endif
#endif

private:
};

}

}
// NOTE: QML/Quick will be used in place of QWidgets/QApplication for its modern and customizable UI rather than going the traditional desktop office-style application route with QWidgets
#endif // MAINWINDOW_H
