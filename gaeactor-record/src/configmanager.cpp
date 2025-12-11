#pragma execution_character_set("utf-8")
#include "configmanager.h"
#include <QCoreApplication>

#include <QJsonArray>

#include <QFileInfo>

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

