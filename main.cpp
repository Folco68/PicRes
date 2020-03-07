#include "DropThread.hpp"
#include "MainWindow.hpp"
#include <QApplication>

//
//  main
//
// Create the MainWindow, show it and execute it
//

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w(argc, argv); // Handle files dropped on the program icon (or passed from CLI)
    w.show();
    int ret = a.exec();
    delete DropThread::instance();
    return ret;
}
