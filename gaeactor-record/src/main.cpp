#include "./gaeactormanager.h"
#include <QCoreApplication>
#ifdef WIN32
#   include <windows.h>
#endif

#include "configmanager.h"
#include "gaeactormanager.h"

#include "settingsconfig.h"
#include "gaeactor_comm_interface.h"
#include <sstream>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//#ifdef WIN32
//	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
//#endif
//	QCoreApplication::setOrganizationName("Some organization");
    QString groupname="default";
    QString base_log_dir="";
    if(argc == 2)
    {
        ConfigManager::getInstance().m_path = a.arguments().at(1);
        GaeactorManager::getInstance().set_usinglocalwsaddr(true);
    }
    else if(argc == 5)
    {
        groupname = a.arguments().at(1);
        GaeactorManager::getInstance().setLocalip(a.arguments().at(2));
        GaeactorManager::getInstance().setHub_websocketport(a.arguments().at(3).toUShort());
        ConfigManager::getInstance().m_path = a.arguments().at(4);
        GaeactorManager::getInstance().set_usinglocalwsaddr(false);
        std::stringstream ss;
        ss <<" -----------------------------record param -----------------------------\n"
           <<" groupname "<<groupname.toStdString()<<" \n"
           <<" ip: "<<GaeactorManager::getInstance().localip().toStdString()<<" \n"
           <<" port: "<<GaeactorManager::getInstance().hub_websocketport()<<" \n"
           <<" -----------------------------------------------------------------------\n";
        std::cout<<ss.str();
    }
    else
    {
        QDateTime datetime = QDateTime::currentDateTime().toUTC();
        QString dateStr = datetime.toString("yyyyMMddhhmmsszzz");
        ConfigManager::getInstance().m_path = "custom_"+dateStr;
        GaeactorManager::getInstance().set_usinglocalwsaddr(true);
    }
    SettingsConfig::getInstance().setGroupname(groupname);

    gaeactorcomm::GaeactorComm::getInstance().init(SettingsConfig::getInstance().getCommserviceName().c_str());
    std::stringstream ss2;
    ss2 <<" record path: "<<ConfigManager::getInstance().m_path.toStdString()<<" \n";
    std::cout<<ss2.str();
    GaeactorManager::getInstance().init();
    GaeactorManager::getInstance().setRunning(true);
	return a.exec();
}
