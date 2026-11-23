#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("WAV");
    QGuiApplication::setWindowIcon(QIcon(":/qt/qml/WAV/rc/icons/wave.ico"));
    QQuickStyle::setStyle("FluentWinUI3");
    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("WAV", "Main");

    return QGuiApplication::exec();
}
