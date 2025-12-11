#include "global_variables.h"
#include <QQuickWidget>
#include <QQmlContext>

int32_t QMLGlobalVariableHelper::ctrl_height = 32;
int32_t QMLGlobalVariableHelper::title_font_size = 36;
int32_t QMLGlobalVariableHelper::subtitle_font_size = 24;
int32_t QMLGlobalVariableHelper::context_font_size = 21;

QMLGlobalVariableHelper::QMLGlobalVariableHelper()
{

}

QMLGlobalVariableHelper::~QMLGlobalVariableHelper()
{

}

void QMLGlobalVariableHelper::setWidgetGlobalVariable(QQuickWidget * qmlWidget)
{
    qmlWidget->rootContext()->setContextProperty("ctrl_height",ctrl_height);
    qmlWidget->rootContext()->setContextProperty("title_font_size",title_font_size);
    qmlWidget->rootContext()->setContextProperty("subtitle_font_size",subtitle_font_size);
    qmlWidget->rootContext()->setContextProperty("context_font_size",context_font_size);
}
