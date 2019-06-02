#include <QtWidgets/QApplication>
#include <QtWidgets>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication *app = new QApplication(argc, argv);
    MainWindow *mainWindow = new MainWindow();
    mainWindow->show();
    return app->exec();
}
