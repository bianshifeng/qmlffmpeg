#ifndef FFMPEG_WORKER_H
#define FFMPEG_WORKER_H

#include <map>
using namespace std;

#include <QObject>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>
#include <QElapsedTimer>

#include "../../qmlffmpeg/Channel.h"


typedef map<string, string> FFMPEGParams;
Q_DECLARE_METATYPE(FFMPEGParams)

class QVideoFrame;
class QVideoSurfaceFormat;

class FFmpegWorker : public QObject, ffmpeg::Channel
{
    Q_OBJECT
public:
    FFmpegWorker();

private:
    QTimer _timer;
    QElapsedTimer _elapsedTimer;
    QByteArray _image;
    QByteArray _sound;
    QAudioOutput* _audioOutput = Q_NULLPTR;
    QIODevice* _audioInput = Q_NULLPTR;
    qint64 _audioTimeStamp = 0;
    qint64 _videoTimeStamp = 0;
    qint64 _timeInterval = 0;
    qreal  _volume = 0;

public:
    virtual bool hasVideo();
    virtual bool hasAudio();

    virtual void lockImage(){}
    virtual void *createImage(int width, int height, int& align);
    virtual void unlockImage(){}

    virtual void lockSound(){}
    virtual void *createSound(AVSampleFormat sampleFormat, int channelCount, int sampleCount, int bitRate, int sampleRate, int& align);
    virtual void unlockSound(){}

    virtual bool interrupt();
    virtual void free();

public slots:
    void toPlay(const QString& source, const FFMPEGParams& params);
    void toStop();
    void toChangeVolume(qreal volume);
    void toUpdate();

signals:
    void sigVolumeChanged(qreal volume);
    void sigBeginVideoSurface(const QVideoSurfaceFormat& format);
    void sigPresentVideoSurface(const QVideoFrame& frame);
    void sigStopVideoSurface();

};

#endif // FFMPEG_WORKER_H

