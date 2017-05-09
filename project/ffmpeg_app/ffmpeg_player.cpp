#include "ffmpeg_player.h"

#include <QVideoFrame>
#include <QAbstractVideoBuffer>
#include <QJSValueIterator>
#include <QJSValue>

void FFmpegPlayer::setSource(QString source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(source);
}

void FFmpegPlayer::setPlaying(bool playing)
{
    if (m_playing == playing)
        return;

    m_playing = playing;
    emit playingChanged(playing);
}

void FFmpegPlayer::setVolume(qreal volume)
{
    if (m_volume == volume)
        return;

    m_volume = volume;
    emit volumeChanged(volume);
}

void FFmpegPlayer::setParams(QJSValue params)
{
    m_params = params;
    m_core_params.clear();
    QJSValueIterator it(params);
    while (it.hasNext()) {

        it.next();
        m_core_params.insert(pair<string,string>(it.name().toStdString(),it.value().toString().toStdString()));
    }
    emit paramsChanged(params);
}


FFmpegPlayer::FFmpegPlayer(QObject *parent) : QObject(parent)
{
    m_worker->moveToThread(&m_thread);
    connect(&m_thread,SIGNAL(finished()),m_worker,SLOT(deleteLater()));

    connect(this,SIGNAL(sigPlay(QString,FFMPEGParams)),m_worker,SLOT(toPlay(QString,FFMPEGParams)));
    connect(this,SIGNAL(sigStop()),m_worker,SLOT(toStop()));
    connect(this,SIGNAL(sigChangeVolume(qreal)),m_worker,SLOT(toChangeVolume(qreal)));

    connect(m_worker,SIGNAL(sigBeginVideoSurface(QVideoSurfaceFormat)),this,SLOT(beginVideoSurface(QVideoSurfaceFormat)));
    connect(m_worker,SIGNAL(sigPresentVideoSurface(QVideoFrame)),this,SLOT(presentVideoSurface(QVideoFrame)));
    connect(m_worker,SIGNAL(sigStopVideoSurface()),this,SLOT(stopVideoSurface()));

    m_thread.start();
}

FFmpegPlayer::~FFmpegPlayer()
{
    this->stop();
    m_thread.quit();
    m_thread.wait();

}

void FFmpegPlayer::play()
{
    emit sigPlay(m_source,m_core_params);

}

void FFmpegPlayer::stop()
{
    emit sigStop();
}

bool FFmpegPlayer::save(const QString &path, int width)
{
    if(m_frame.map(QAbstractVideoBuffer::ReadOnly))
    {
        QImage t_image(m_frame.bits(),m_frame.width(),m_frame.height(),m_frame.bytesPerLine(),QImage::Format_RGBA8888);
        const_cast<QVideoFrame&>(m_frame).unmap();
        return t_image.scaledToWidth(width? width: m_frame.width(),Qt::SmoothTransformation).save(path);
    }
    return false;
}

void FFmpegPlayer::beginVideoSurface(const QVideoSurfaceFormat &format)
{
    this->setPlaying(false);
    if(!m_videoSurface->start(format))
    {
        emitVideoSurfaceError();
    }
}

void FFmpegPlayer::presentVideoSurface(const QVideoFrame &frame)
{
    m_frame = frame;
    if(m_videoSurface->present(frame))
    {
        this->setPlaying(true);
    }else{
        emitVideoSurfaceError();
    }
}

void FFmpegPlayer::stopVideoSurface()
{
    this->setPlaying(false);
    m_videoSurface->stop();
}

void FFmpegPlayer::emitVideoSurfaceError()
{
    switch (m_videoSurface->error())
    {
    case QAbstractVideoSurface::UnsupportedFormatError: emit sigError("video format was not supported"); break;
    case QAbstractVideoSurface::IncorrectFormatError: emit sigError("video frame was not compatible with the format of the surface"); break;
    case QAbstractVideoSurface::StoppedError: emit sigError("surface has not been started"); break;
    case QAbstractVideoSurface::ResourceError: emit sigError("surface could not allocate some resource"); break;
    }

}

