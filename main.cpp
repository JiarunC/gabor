#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDebug>
#include <iostream>
#include "config.h"
#include "mkl.h"

void test() {
    
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("WAV");
    QQuickStyle::setStyle("FluentWinUI3");
    QQmlApplicationEngine engine;

    engine.setExternalSingletonInstance("Constants","Singleton",&Config::instance());
    engine.loadFromModule("Constants", "Main");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("WAV", "Main");

    qDebug() << "Starting";

    return QGuiApplication::exec();
}
