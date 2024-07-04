#include "mainwindow.h"
#include "pc.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    pc p;
    w.show();
    p.show();
    return a.exec();
}
