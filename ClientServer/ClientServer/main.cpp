#include <QtCore/QCoreApplication>
#include "ClientServer.h"

int main(int argc, char *argv[])
{
    HWND console = GetConsoleWindow();

    if (console)
        ShowWindow(console, SW_HIDE); //SW_SHOW - показать
    QCoreApplication a(argc, argv);
    
    ClientServer server;
    return a.exec();
}
