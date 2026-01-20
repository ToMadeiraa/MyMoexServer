#include <QCoreApplication>
#include "MyServer.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << argc;
    qDebug() << argv[1];
    qDebug() << argv[2];
    qDebug() << argv[3];
    qDebug() << argv[4];
    Server server;
    return a.exec();
}



