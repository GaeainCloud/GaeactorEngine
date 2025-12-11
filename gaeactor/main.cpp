
#ifndef USING_GAEACTOR_EXPORT_LIB

#include <QCoreApplication>
#include "gaeactortransmitmanager.h"
#include "loghelper.h"

#include "easy/profiler.h"
#ifdef _MSC_VER
#include <windows.h>
#endif
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    profiler::startListen(29362);
    //修改进程优先级
#ifdef _MSC_VER
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif

    log_service::LogHelper::instance()->initLog(QCoreApplication::applicationName());
    if(argc == 2)
    {
        GaeactorTransmitManager::getInstance(a.arguments().at(1)).set_transmit_channel_id(QCoreApplication::applicationName()+"_"+a.arguments().at(1));
    }
    else
    {
        GaeactorTransmitManager::getInstance().set_transmit_channel_id("");
    }
    return a.exec();
}

#endif
