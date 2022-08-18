#include "main_window.hpp"

#if defined(NEROSHOP_USE_QT)
#include "ui_main_window.h" // requires that CMAKE_AUTO*** be turned on
#endif

neroshop::gui::MainWindow::MainWindow() {
#if defined(NEROSHOP_USE_QT)
    this->setCentralWidget(new QWidget());
#endif    
}

/*gui::MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    //ui->setupUi(this);
}*/

neroshop::gui::MainWindow::~MainWindow() {
    //delete ui;
    std::cout << "main window deleted\n";
}

#if defined(NEROSHOP_USE_QT)
void neroshop::gui::MainWindow::on_actionExit_triggered() {
    this->close();
}
#endif
