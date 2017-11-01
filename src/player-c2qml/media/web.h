#ifndef WEB_H
#define WEB_H

#include "smilparser/web.h"
#include "base_media.h"

class Web : public BaseMedia
{
        Q_OBJECT
    public:
        Web(TMedia *media, QObject *parent = nullptr);
        ~Web();
        void          init(QQmlComponent *mc);
        void          setParentItem(QQuickItem *parent);
    protected:
        TWeb          *MyWeb;
        QScopedPointer<QQuickItem>   web_item;
};

#endif // WEB_H