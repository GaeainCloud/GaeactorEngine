#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "params_define.h"
#ifdef USING_GUI_SHOW
#include <QMainWindow>
#include <QHBoxLayout>
#else
#include <QObject>
#endif

#include "./httpserver/httpserver/dto/AgentDto.hpp"

#include <set>

#ifdef USING_GUI_SHOW
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class Map2dWidget;
class Map2dEditWidget;
class QtOSGWidget;
#endif

class HttpServer;
namespace stdutils {
class OriThread;
}

#include <QColor>
#include <vector>

class ConcurrentHashMapManager;

#ifdef USING_GUI_SHOW
class MainWindow : public QMainWindow
#else
class MainWindow : public QObject
#endif
{
    Q_OBJECT

public:
    static MainWindow & getInstance();
    virtual ~MainWindow();

    void start_HttpServer();
    void stop_HttpServer();

    ConcurrentHashMapManager *pConcurrentHashMapManager() const;

private:
    void drawHex_ex(const TYPE_ULID &uildsrc, const HEXIDX_HGT_ARRAY& hexidxslist, const std::vector<QColor> &clstmp = std::vector<QColor>());

    void dreaw_result(UINT64 task_id, GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance);

private slots:
    void draw_linestring_slot(tagLineInfo jsobj);
    void deal_result_slot(const QString &jsobj, void *ptr);

private:
#ifdef USING_GUI_SHOW
    MainWindow(QWidget *parent = nullptr);
    Ui::MainWindow *ui;
#else
    MainWindow(QObject *parent = nullptr);
#endif

    bool httpdatareceive_callback(E_DATA_TYPE eDataType, const QJsonObject & obj);
    void thread_httpserver_callback_Loop(void* param);

private:

#ifdef USING_GUI_SHOW
    QHBoxLayout *m_pLayout;
    Map2dWidget* m_pMapWidget;
    QtOSGWidget* m_pModelWidget;
#endif
    std::set<quint64> m_drawlineids;

    HttpServer* m_phttpserver;
    stdutils::OriThread *m_pHttpServerRunningThread;

private:

    ConcurrentHashMapManager* m_pConcurrentHashMapManager;

};
#endif // MAINWINDOW_H
