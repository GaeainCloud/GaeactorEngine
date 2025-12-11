#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>

class QIODevice;
class ConfigManager:public QObject
{
    Q_OBJECT
public:
    static ConfigManager & getInstance();
    virtual ~ConfigManager();

private:
    ConfigManager(QObject *parent = nullptr);
    void init();
public:
    QString m_path;
};

#endif // CONFIGMANAGER_H
