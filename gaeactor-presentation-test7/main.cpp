#include "mainwindow.h"
#ifdef USING_GUI_SHOW
#include <QApplication>
#else
#include <QCoreApplication>
#endif
#ifdef WIN32
#include "crashhelper.h"
#include <windows.h>
#endif
#include "easy/profiler.h"

#include "loghelper.h"
int main(int argc, char *argv[])
{
#ifdef USING_GUI_SHOW
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);
#else
    QCoreApplication a(argc, argv);
#endif


    QString moduleName = "gaeactor-presentation-test7";

#ifdef USING_GUI_SHOW
    moduleName = "Deconflictor_gui";
#else
    moduleName = "Deconflictor";
#endif

    log_service::LogHelper::instance(moduleName)->initLog(moduleName);
#ifdef Q_OS_WIN32
    // dump
    gaeactor_dump::CrashDumpHelper::setDump(moduleName);
#elif defined(Q_OS_LINUX)
    ///TODO:Linux
#endif
    profiler::startListen(29367);
    MainWindow::getInstance().start_HttpServer();

#ifdef USING_GUI_SHOW
    MainWindow::getInstance().show();
#endif
    return a.exec();
}


