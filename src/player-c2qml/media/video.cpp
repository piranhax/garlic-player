#include "video.h"

Video::Video(QQmlComponent *mc, QString r_id, QObject *parent) : BaseMedia(mc, r_id, parent)
{
    setRegionId(r_id);
#ifdef SUPPORT_QTAV
    QString module = "import QtAV 1.7\n";
#else
    QString module = "import QtMultimedia 5.7\n";
#endif
    QString str("import QtQuick 2.7\n"+
                    module +
                    "Video { \
                        id: "+getRegionId()+"_video; \
                        anchors.fill: parent; \
                        autoPlay: true; \
                        property var video_fill_mode: \"\";  \
                   }\n"
    );
// do not work in Android cause "parent" and "this" returns NULL
//onStatusChanged:
//{
//    parent.parent.fitVideo(this, video_fill_mode);
//}
    video_item.reset(createMediaItem(mc, str));
    connect(video_item.data(), SIGNAL(stopped()), this, SLOT(finished()));
}

Video::~Video()
{
    video_item.reset();
}

void Video::init(TMedia *media)
{
    MyMedia = media;
    if (load(video_item.data()))
    {
        TVideo *MyVideo = qobject_cast<TVideo *> (media);
        float vol = determineVolume(MyVideo->getSoundLevel());
        video_item.data()->setProperty("volume", vol);
        video_item.data()->setProperty("fillMode", determineFillMode(MyMedia->getFit()));
        if (MyMedia->getLogContentId() != "")
            setStartTime();
    }
}

void Video::deinit()
{
    if (MyMedia->getLogContentId() != "")
        qInfo(PlayLog).noquote() << createPlayLogXml();
    video_item.data()->setProperty("source", "");
}

void Video::setParentItem(QQuickItem *parent)
{
    video_item.data()->setParentItem(parent);
}

int Video::determineFillMode(QString smil_fit)
{
    // FIXME Look if this code can set in Base Class
    if (smil_fit == "fill")
        return STRETCH;
    else if (smil_fit == "meet")
        return PRESERVEASPECTCROP;
    else if (smil_fit == "meetbest")
        return PRESERVEASPECTFIT;
    else
        return STRETCH;
}

qreal Video::determineVolume(QString percent)
{
    qreal vol = 0;
    if (percent.endsWith('%'))
        vol = percent.mid(0, percent.length()-1).toFloat();

    return vol / (float) 100;
}

void Video::finished()
{
    MyMedia->finishedSimpleDuration();
}

