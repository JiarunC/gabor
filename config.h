#pragma once

#include <QObject>
#include <QFontDatabase>
#include <QQmlEngine>
#include <QDirIterator>
#include <QStandardPaths>

class Config : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT

    Q_PROPERTY(QString bg_color READ bg_color CONSTANT)
    Q_PROPERTY(int common_margin READ common_margin CONSTANT)
    Q_PROPERTY(int common_padding READ common_padding CONSTANT)
    Q_PROPERTY(int common_element_radius READ common_element_radius CONSTANT)
    Q_PROPERTY(int common_container_radius READ common_container_radius CONSTANT)
    Q_PROPERTY(int common_element_height READ common_element_height CONSTANT)
    Q_PROPERTY(QString label_font READ label_font CONSTANT)
    Q_PROPERTY(QString info_font READ info_font CONSTANT)
    Q_PROPERTY(QString primary_text_color READ primary_text_color CONSTANT)
    Q_PROPERTY(QString vibrant_warm_text_color READ vibrant_warm_text_color CONSTANT)
    Q_PROPERTY(QString vibrant_cool_text_color READ vibrant_cool_text_color CONSTANT)
    Q_PROPERTY(QString warm_bg_color READ warm_bg_color CONSTANT)
    Q_PROPERTY(QString secondary_warm_bg_color READ secondary_warm_bg_color CONSTANT)
    Q_PROPERTY(QString cool_bg_color READ cool_bg_color CONSTANT)
    Q_PROPERTY(QString secondary_cool_bg_color READ secondary_cool_bg_color CONSTANT)

public:
    explicit Config(QObject *parent = nullptr) : QObject(parent) {
        //QFontDatabase::addApplicationFont(":/qt/qml/Leaves/rc/fonts/Alice/Alice-Regular.ttf");
        //QFontDatabase::addApplicationFont(":/qt/qml/Leaves/rc/fonts/Nunito/static/Nunito-Regular.ttf");
        //QFontDatabase::addApplicationFont(":/qt/qml/Leaves/rc/fonts/Nunito/static/Nunito-Bold.ttf");
        //QFontDatabase::addApplicationFont(":/qt/qml/Leaves/rc/fonts/Federo/Federo-Regular.ttf");
    }
    static Config& instance() {
        static Config _instance;
        return _instance;
    }
    static QString bg_color() {
        return "#282529";
    }
    static int common_margin() {
        return 50;
    }
    static int common_padding() {
        return 6;
    }
    static int common_element_radius() {
        return 6;
    }
    static int common_container_radius() {
        return 10;
    }
    static int common_element_height() {
        return 20;
    }
    static QString label_font() {
        return "Alice";
    }
    static QString info_font() {
        return "Nunito";
    }
    static QString primary_text_color() {
        return "ivory";
    }
    static QString vibrant_warm_text_color() {
        return "mediumvioletred";
    }
    static QString vibrant_cool_text_color() {
        return "darkturquoise";
    }
    static QString warm_bg_color() {
        return "#40333a";
    }
    static QString secondary_warm_bg_color() {
        return "#715966";
    }
    static QString cool_bg_color() {
        return "#344350";
    }
    static QString secondary_cool_bg_color() {
        return "#576977";
    }
};
