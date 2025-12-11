#include "mainwindow.h"

#include <QApplication>
#include "loghelper.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    log_service::LogHelper::instance()->initLog("gaeactor-viewer");
    MainWindow w;
    w.show();
    w.showMaximized();
    return a.exec();
}
