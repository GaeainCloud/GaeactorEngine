#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <stdint.h>

class QQuickWidget;
class QMLGlobalVariableHelper
{
public:
    QMLGlobalVariableHelper();
    ~QMLGlobalVariableHelper();


    static void setWidgetGlobalVariable(QQuickWidget * qmlWidget);

    static int32_t ctrl_height;
    static int32_t title_font_size;
    static int32_t subtitle_font_size;
    static int32_t context_font_size;
};



#endif
