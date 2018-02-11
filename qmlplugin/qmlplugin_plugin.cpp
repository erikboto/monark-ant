#include "qmlplugin_plugin.h"
#include "monark.h"

#include <qqml.h>

void QmlpluginPlugin::registerTypes(const char *uri)
{
    // @uri se.unixshell.monark
    qmlRegisterType<Monark>(uri, 1, 0, "Monark");
}

