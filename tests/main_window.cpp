#include "main_window.hpp"
#include "ui_main_window.h" // requires that CMAKE_AUTO*** be turned on

gui::MainWindow::MainWindow() {
    this->setCentralWidget(new QWidget());
}

/*gui::MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new gui::MainWindow)
{
    //ui->setupUi(this);
}*/

gui::MainWindow::~MainWindow() {
    //delete ui;
    std::cout << "main window deleted\n";
}

void gui::MainWindow::on_actionExit_triggered()
{
    this->close();
}
