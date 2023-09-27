#include <QtCore/QCoreApplication>
#include "ClientServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ClientServer server;
    return a.exec();
}
