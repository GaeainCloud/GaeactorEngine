#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <QObject>

#define HTTP_PORT (18089)
#define LOG_PORT (18090)
#define REVIEW_PORT (18091)
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
    uint32_t step_interval;
    double step_dt;
    double step_freq;
    double one_second_sim_step_second;


    uint32_t review_step_interval;
    double review_step_dt;
    double review_step_freq;
    double review_one_second_sim_step_second;
};

#endif // CONFIGMANAGER_H
