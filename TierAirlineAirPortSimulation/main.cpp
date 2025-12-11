
#include "./src/uiwidget/widgetmanager.h"
#include <QCoreApplication>
#include <QTranslator>
#ifdef WIN32
#include "crashhelper.h"
#include <windows.h>
#endif
#include "LocationHelper.h"
#include "easy/profiler.h"

#include "settingsconfig.h"
#include "loghelper.h"
#include "runningmodeconfig.h"
int main(int argc, char *argv[])
{
//	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication a(argc, argv);

    QString groupname ="default";
    switch (argc)
    {
    case 2:
    {
        groupname = a.arguments().at(1);
    }
    break;
    }

    SettingsConfig::getInstance().setGroupname(groupname);
    gaeactorcomm::GaeactorComm::getInstance().init(SettingsConfig::getInstance().getCommserviceName().c_str());

    runningmode::RunningModeConfig::getInstance().load();

    log_service::LogHelper::instance("lavaic-desktop")->initLog("TierAirlineAirPortSimulation");
//    profiler::startListen(29364);

#ifdef Q_OS_WIN32
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    // dump
    gaeactor_dump::CrashDumpHelper::setDump("TierAirlineAirPortSimulation");
#elif defined(Q_OS_LINUX)
    ///TODO:Linux
#endif

	QCoreApplication::setOrganizationName("Some organization");


    runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");

	WidgetManager w;
    w.run();
	return a.exec();
}
