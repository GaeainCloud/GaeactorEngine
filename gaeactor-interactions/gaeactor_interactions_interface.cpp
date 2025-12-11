#include "gaeactor_interactions_processor.h"
#include "gaeactor_interactions_interface.h"
#include <QDebug>
namespace gaeactorinteractions
{
GaeactorInteractions::GaeactorInteractions(QObject *parent)
    :QObject(parent)
{

}

GaeactorInteractions::~GaeactorInteractions()
{

}

void GaeactorInteractions::setCheckEnable(bool newBChecking)
{
    GaeactorInteractionsProcessor::getInstance().setCheckEnable(newBChecking);
}

void GaeactorInteractions::refreshInteractions(int id)
{
    GaeactorInteractionsProcessor::getInstance().refreshInteractions(id);
}

void GaeactorInteractions::registDisplayCallback(echowave_display_hexidx_update_callback func)
{
    GaeactorInteractionsProcessor::getInstance().registDisplayCallback(func);
}

void GaeactorInteractions::registDisplayListCallback(echowave_list_display_hexidx_update_callback func)
{
    GaeactorInteractionsProcessor::getInstance().registDisplayListCallback(func);
}

void GaeactorInteractions::registHexidxDisplayCallback(display_hexidx_update_callback func)
{
    GaeactorInteractionsProcessor::getInstance().registHexidxDisplayCallback(func);
}

GaeactorInteractions &GaeactorInteractions::getInstance()
{
    static GaeactorInteractions gaeactorinteractions;
    return gaeactorinteractions;
}

}
