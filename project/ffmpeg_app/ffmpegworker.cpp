#include "ffmpeg_worker.h"

#include <QVideoFrame>
#include <QAbstractVideoBuffer>
#include <QVideoSurfaceFormat>

#include "../../qmlffmpeg/Client.h"


FFmpegWorker::FFmpegWorker():ffmpeg::Channel(AVPixelFormat::AV_PIX_FMT_RGBA,AV_SAMPLE_FMT_S16),
    _timer(this)
{
    _timer.setInterval(1);
    _timer.setSingleShot(false);
    connect(&_timer,SIGNAL(timeout()),this,SLOT(toUpdate()));
}

void *FFmpegWorker::createImage(int width, int height, int &align)
{
    QVideoSurfaceFormat format(QSize(width,height), QVideoFrame::Format_BGR32);

    qreal t_rate = this->videoCodec->time_base.den / this->videoCodec->time_base.num;
    format.setFrameRate(t_rate);

    emit sigBeginVideoSurface(format);

    _image.resize(width*height*this->getBits()/8);

    return _image.data();
}

void *FFmpegWorker::createSound(AVSampleFormat sampleFormat, int channelCount, int sampleCount, int bitRate, int sampleRate, int &align)
{
    _timeInterval = 1000 * sampleCount / sampleRate;
    _sound.resize(av_samples_get_buffer_size(nullptr,channelCount,sampleCount,sampleFormat, align));
    QAudioFormat format;
    format.setChannelCount(channelCount);
    format.setCodec("audio/pcm");
    format.setSampleRate(sampleRate);
    format.setSampleSize(bitRate);
    switch(sampleFormat)
    {
    case AV_SAMPLE_FMT_U8:
    case AV_SAMPLE_FMT_U8P: format.setSampleType(QAudioFormat::UnSignedInt); break;
    case AV_SAMPLE_FMT_S16:
    case AV_SAMPLE_FMT_S32:
    case AV_SAMPLE_FMT_S16P:
    case AV_SAMPLE_FMT_S32P: format.setSampleType(QAudioFormat::SignedInt); break;
    case AV_SAMPLE_FMT_FLT:
    case AV_SAMPLE_FMT_DBL:
    case AV_SAMPLE_FMT_FLTP:
    case AV_SAMPLE_FMT_DBLP: format.setSampleType(QAudioFormat::Float); break;
    default: format.setSampleType(QAudioFormat::Unknown);
    }

    _audioOutput = new QAudioOutput(format);
    _audioInput = _audioOutput->start();

    return _sound.data();
}

bool FFmpegWorker::interrupt()
{
    return false;
}

void FFmpegWorker::toUpdate()
{
    this->receive();

    if( 0 == _audioTimeStamp) _elapsedTimer.restart(); //开始时间同步

    if( 0 == _timeInterval && hasVideo()){
        _timeInterval = 1000 * this->videoCodec->time_base.num / this->videoCodec->time_base.den;
    }

    if(_elapsedTimer.elapsed() >= _audioTimeStamp)
    {
        if(hasAudio())
        {
            qint64 t_timeStamp = this->popAudioSample();
            if(t_timeStamp >= 0)
            {
                _audioTimeStamp = t_timeStamp;
                //_audioInput->write(_sound.data(),_sound.size());
            }
        }
    }else{
        _audioTimeStamp += _timeInterval;
    }

    if(hasVideo()){
        while (_videoTimeStamp < _audioTimeStamp) {
            int t_timeStamp = this->popVideoFrame();
            if(t_timeStamp >= 0)
            {
                _videoTimeStamp = t_timeStamp;

                QVideoFrame frame(_image.size(),QSize(this->videoCodec->width,this->videoCodec->height),_image.size()/this->videoCodec->height,QVideoFrame::Format_BGR32);
                if(frame.map(QAbstractVideoBuffer::WriteOnly))
                {
                    memcpy(frame.bits(),_image.data(),_image.size());
                    frame.unmap();
                    emit sigPresentVideoSurface(frame);
                }
            }else{
                break;
            }
        }
    }


    qreal t_volume = hasAudio() ? _audioOutput->volume(): -1;

    if(t_volume != _volume)
    {
        _volume = t_volume;
        emit sigVolumeChanged(t_volume);
    }
}

void FFmpegWorker::toPlay(const QString &source, const FFMPEGParams &params)
{
    this->reset(source.toStdString().c_str(),params);
    _videoTimeStamp = _audioTimeStamp = 0;
    _timer.start();
}

void FFmpegWorker::toStop()
{
    _timer.stop();
    emit sigStopVideoSurface();
}

void FFmpegWorker::toChangeVolume(qreal volume)
{
    if(_audioOutput)
    {
        _audioOutput->setVolume(volume);
    }
}


bool FFmpegWorker::hasVideo()
{

    return ffmpeg::Client::hasVideo() && this->outputVideo;
}

bool FFmpegWorker::hasAudio()
{
    return ffmpeg::Client::hasVideo() && this->outputAudio;
}

void FFmpegWorker::free()
{
    ffmpeg::Channel::free();
    if(_audioOutput) delete _audioOutput;
}
