import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import Overlay
import Config
import QtQuick.Effects

ApplicationWindow {
    id: launcher_window
    color: Config.bg_color
    minimumWidth: 256
    minimumHeight: 358
    maximumWidth: 256
    maximumHeight: 358
    visible: true
    title: qsTr("Launcher")

    property int overlay_resolution: 1024
    property int overlay_interval: 80
    property int overlay_target_points: 55
    property var overlay_instance: null

    function toggleOverlay() {
        if (!overlay_instance) {
            overlay_instance = overlay_component.createObject(null, {
                process_resolution: Qt.binding(function() { return launcher_window.overlay_resolution; }),
                process_interval: Qt.binding(function() { return launcher_window.overlay_interval; }),
                process_target_points: Qt.binding(function() { return launcher_window.overlay_target_points; })
            });
            if (overlay_instance) {
                overlay_instance.closing.connect(function() {
                    var inst = overlay_instance;
                    overlay_instance = null;
                    Qt.callLater(function() {
                        if (inst) {
                            inst.destroy();
                        }
                    });
                });
            }
        } else {
            overlay_instance.uninitialize();
        }
    }

    onClosing: {
        if (overlay_instance) {
            overlay_instance.close();
            overlay_instance.destroy();
            overlay_instance = null;
        }
    }

    Component {
        id: overlay_component
        Overlay {
            process_resolution: launcher_window.overlay_resolution
            process_interval: launcher_window.overlay_interval
            process_target_points: launcher_window.overlay_target_points
        }
    }

    // Outer matte frame using Config colors
    Rectangle {
        id: outer_card
        anchors.centerIn: parent
        width: parent.width - Config.common_margin
        height: parent.height - Config.common_margin
        radius: Config.common_container_radius
        color: Config.secondary_cool_bg_color
        border.width: 1
        border.color: Config.secondary_cool_bg_color

        // Inner recessed matte container
        Rectangle {
            id: inner_card
            anchors.fill: parent
            anchors.margins: Config.common_padding
            radius: Config.common_container_radius - 4
            color: Qt.darker(Config.cool_bg_color, 2)
            border.width: 1
            border.color: Config.secondary_cool_bg_color

            Column {
                spacing: 12
                anchors.centerIn: parent
                width: parent.width - 28

                CustomComboBox {
                    id: resolution_combo_box
                    alternate_colors: false
                    font.family: Config.info_font
                    font.pixelSize: 13
                    width: 108
                    anchors.horizontalCenter: parent.horizontalCenter
                    model: ["1024", "2048", "4096", "8192", "16384", "32768"]
                    onActivated: {
                        launcher_window.overlay_resolution = parseInt(currentText)
                    }
                }

                Column {
                    width: parent.width
                    spacing: 4

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 6
                        Label {
                            font.family: Config.info_font
                            font.pixelSize: 12
                            color: Config.primary_text_color
                            text: qsTr("Interval")
                        }
                        Label {
                            font.family: Config.info_font
                            font.pixelSize: 13
                            color: Config.vibrant_cool_text_color
                            text: Math.round(interval_slider.value) + " ms"
                        }
                    }

                    Slider {
                        id: interval_slider
                        font.family: Config.info_font
                        width: parent.width
                        from: 30
                        value: 80
                        to: 200
                        stepSize: 1

                        background: Rectangle {
                            x: interval_slider.leftPadding
                            y: interval_slider.topPadding + interval_slider.availableHeight / 2 - height / 2
                            implicitWidth: 140
                            implicitHeight: 6
                            width: interval_slider.availableWidth
                            height: implicitHeight
                            radius: 3
                            color: Config.bg_color
                            border.width: 1
                            border.color: Config.secondary_cool_bg_color

                            Rectangle {
                                width: interval_slider.visualPosition * parent.width
                                height: parent.height
                                radius: 3
                                color: Config.secondary_cool_bg_color
                            }
                        }

                        handle: Rectangle {
                            x: interval_slider.leftPadding + interval_slider.visualPosition * (interval_slider.availableWidth - width)
                            y: interval_slider.topPadding + interval_slider.availableHeight / 2 - height / 2
                            implicitWidth: 14
                            implicitHeight: 14
                            radius: 7
                            color: interval_slider.pressed ? Config.vibrant_cool_text_color : Config.secondary_cool_bg_color
                            border.width: 1
                            border.color: Config.primary_text_color
                        }

                        onMoved: {
                            launcher_window.overlay_interval = value;
                        }
                    }
                }

                Column {
                    width: parent.width
                    spacing: 4

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 6
                        Label {
                            font.family: Config.info_font
                            font.pixelSize: 12
                            color: Config.primary_text_color
                            text: qsTr("Points")
                        }
                        Label {
                            font.family: Config.info_font
                            font.pixelSize: 13
                            color: Config.vibrant_cool_text_color
                            text: Math.round(target_points_slider.value)
                        }
                    }

                    Slider {
                        id: target_points_slider
                        font.family: Config.info_font
                        width: parent.width
                        from: 50
                        value: 60
                        to: 200
                        stepSize: 1

                        background: Rectangle {
                            x: target_points_slider.leftPadding
                            y: target_points_slider.topPadding + target_points_slider.availableHeight / 2 - height / 2
                            implicitWidth: 140
                            implicitHeight: 6
                            width: target_points_slider.availableWidth
                            height: implicitHeight
                            radius: 3
                            color: Config.bg_color
                            border.width: 1
                            border.color: Config.secondary_cool_bg_color

                            Rectangle {
                                width: target_points_slider.visualPosition * parent.width
                                height: parent.height
                                radius: 3
                                color: Config.secondary_cool_bg_color
                            }
                        }

                        handle: Rectangle {
                            x: target_points_slider.leftPadding + target_points_slider.visualPosition * (target_points_slider.availableWidth - width)
                            y: target_points_slider.topPadding + target_points_slider.availableHeight / 2 - height / 2
                            implicitWidth: 14
                            implicitHeight: 14
                            radius: 7
                            color: target_points_slider.pressed ? Config.vibrant_cool_text_color : Config.secondary_cool_bg_color
                            border.width: 1
                            border.color: Config.primary_text_color
                        }

                        onMoved: {
                            launcher_window.overlay_target_points = value;
                        }
                    }
                }

                Item {
                    width: 80
                    height: 80
                    anchors.horizontalCenter: parent.horizontalCenter

                    Image {
                        id: icon
                        source: "rc/images/wave.png"
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        mipmap: true
                        visible: false
                    }

                    MultiEffect {
                        id: image_effect
                        property bool show_effect: true
                        source: icon
                        anchors.centerIn: icon
                        width: icon.width
                        height: icon.height
                        shadowEnabled: true
                        shadowColor: "black"
                        shadowOpacity: 0.7
                        shadowVerticalOffset: show_effect ? 12 : 6
                        shadowScale: 1.02

                        Behavior on shadowVerticalOffset {
                            PropertyAnimation {
                                target: image_effect
                                property: "shadowVerticalOffset"
                                duration: 550
                                easing.type: Easing.InCubic
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: {
                            image_effect.show_effect = false
                        }
                        onExited: {
                            image_effect.show_effect = true
                        }
                        Shortcut {
                            sequence: "Ctrl+Alt+A"
                            onActivated: {
                                launcher_window.toggleOverlay()
                            }
                        }
                        onClicked: {
                            launcher_window.toggleOverlay()
                        }
                    }
                }
            }
        }
    }
}