#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "ffmpeg_player.h"
#include "ffmpeg_worker.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qRegisterMetaType<FFMPEGParams>("FFParams");
    qmlRegisterType<FFmpegPlayer>("Bian", 1, 0, "FFPlayer");


    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    return app.exec();
}
