#ifndef MAIN_WINDOW_HPP_NEROSHOP
#define MAIN_WINDOW_HPP_NEROSHOP

#include <iostream>
#include <QMainWindow>

namespace neroshop {

namespace gui {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    //explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

private:
    //gui::MainWindow *ui;
};

}

}

#endif // MAINWINDOW_H
