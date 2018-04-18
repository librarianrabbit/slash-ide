#include "mainwindow.h"
#include <QApplication>

// TODO: Make cross-platform
#pragma comment(lib, "qscintilla2.lib")

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/slash/slash.ico"));

    MainWindow w;
    w.show();
    w.newTab();
    
    return a.exec();
}
