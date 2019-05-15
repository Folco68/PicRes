#include <QApplication>
#include "MainWindow.hpp"


//
// Create the MainWindow, show it and execute it
//

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
