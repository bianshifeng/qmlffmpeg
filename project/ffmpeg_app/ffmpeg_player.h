#ifndef FFMPEG_PLAYER_H
#define FFMPEG_PLAYER_H

#include <QObject>
#include <QThread>
#include <QAbstractVideoSurface>
#include <QJSValue>
#include "ffmpeg_worker.h"

class FFmpegPlayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QAbstractVideoSurface* videoSurface MEMBER m_videoSurface)
    Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QJSValue params READ params WRITE setParams NOTIFY paramsChanged)
    Q_DISABLE_COPY(FFmpegPlayer)
public:
    QString source() const{return m_source;}
    bool playing() const{return m_playing;}
    qreal volume() const{return m_volume;}
    QJSValue params() const {return m_params;}

signals:
    void sourceChanged(QString source);
    void playingChanged(bool playing);
    void volumeChanged(qreal volume);
    void paramsChanged(QJSValue params);

public slots:
    void setSource(QString source);
    void setPlaying(bool playing);
    void setVolume(qreal volume);
    void setParams(QJSValue params);

private:
    QString m_source;
    bool m_playing;
    qreal m_volume;
    QJSValue m_params;
    QAbstractVideoSurface* m_videoSurface;

public:
    explicit FFmpegPlayer(QObject *parent = 0);
    ~FFmpegPlayer();

signals:
    void sigPlay(const QString& source, const FFMPEGParams& params);
    void sigStop();
    void sigChangeVolume(qreal volume);
    void sigError(const QString& error);

public slots:
    void play();
    void stop();
    bool save(const QString& path, int width = 0);

    void beginVideoSurface(const QVideoSurfaceFormat& format);
    void presentVideoSurface(const QVideoFrame& frame);
    void stopVideoSurface();

private:
    FFmpegWorker* m_worker = new FFmpegWorker;
    FFMPEGParams m_core_params;
    QThread m_thread;
    QVideoFrame m_frame;


    void emitVideoSurfaceError();

};

#endif // FFMPEG_PLAYER_H
