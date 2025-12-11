#pragma execution_character_set("utf-8")
#include "configmanager.h"
#include <QCoreApplication>
#include "function.h"
#define WEBSOCKET_PORT (31769)
ConfigManager &ConfigManager::getInstance()
{
    static ConfigManager configmanager;
    return configmanager;
}

ConfigManager::~ConfigManager()
{

}


ConfigManager::ConfigManager(QObject *parent)
{
    init();
}

void ConfigManager::init()
{
}


