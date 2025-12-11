#include "gaeactor_auditions_processor.h"
#include "gaeactor_auditions_interface.h"
#include <QDebug>
namespace gaeactorauditions
{
GaeactorAuditions::GaeactorAuditions(QObject *parent)
    :QObject(parent)
{

}

GaeactorAuditions::~GaeactorAuditions()
{

}

void GaeactorAuditions::setCheckEnable(bool newBChecking)
{
    GaeactorAuditionsProcessor::getInstance().setCheckEnable(newBChecking);
}

void GaeactorAuditions::refreshAuditions()
{
    GaeactorAuditionsProcessor::getInstance().refreshAuditions();
}

void GaeactorAuditions::registDisplayCallback(echowave_display_hexidx_update_callback func)
{
    GaeactorAuditionsProcessor::getInstance().registDisplayCallback(func);
}

void GaeactorAuditions::registHexidxDisplayCallback(display_hexidx_update_callback func)
{
    GaeactorAuditionsProcessor::getInstance().registHexidxDisplayCallback(func);
}

GaeactorAuditions &GaeactorAuditions::getInstance()
{
    static GaeactorAuditions gaeactorauditions;
    return gaeactorauditions;
}

}
