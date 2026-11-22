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

    Loader {
        id: app_loader
        sourceComponent: undefined
        onLoaded: {
            if (item) {
                item.closing.connect(function () {
                    Qt.callLater(function () {
                        app_loader.sourceComponent = undefined;
                    });
                });
            }
        }
    }

    Component {
        Overlay {
            process_resolution: launcher_window.overlay_resolution
        }
        id: overlay_component
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width - Config.common_margin
        height: parent.height - Config.common_margin
        radius: 18
        gradient: Gradient {
            GradientStop { position: .3; color: "#005B5E12" }
            GradientStop { position: 1; color: "#952e94" }
        }
        visible: true



        Column {
            spacing: 24
            anchors.centerIn: parent

            CustomComboBox {
                id: resolution_combo_box
                visible: !app_loader.item
                alternate_colors: false
                font.family: Config.input_font
                width: 80
                anchors.horizontalCenter: parent.horizontalCenter
                model: ["1024", "2048", "4096", "8192", "16384", "32768"]
                onActivated: {
                    launcher_window.overlay_resolution = parseInt(currentText)
                }
            }

            Rectangle {
                width: 80
                height: 80
                anchors.horizontalCenter: parent.horizontalCenter
                color: "transparent"
                Image {
                    id: icon
                    source: "rc/images/CL.png"
                    anchors.fill:parent
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    mipmap: true

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            image_effect.show_effect = false
                        }
                        onExited: {
                            image_effect.show_effect = true
                        }
                        Shortcut {
                            sequence: "Ctrl+Alt+A"

                            onActivated: {
                                if (!app_loader.sourceComponent) {
                                    app_loader.sourceComponent = overlay_component
                                } else if (app_loader.sourceComponent === overlay_component) {
                                    // destroy but not close, overlay closes itself
                                    app_loader.item.uninitialize()
                                }
                            }
                        }
                        onClicked: {
                            if (!app_loader.sourceComponent) {
                                app_loader.sourceComponent = overlay_component
                            } else if (app_loader.sourceComponent === overlay_component) {
                                // destroy but not close, overlay closes itself
                                app_loader.item.uninitialize()
                            }
                        }
                    }
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
                    shadowOpacity: .7
                    shadowVerticalOffset: show_effect ? 12 : 6
                    shadowScale: 1.02
                    opacity: 1

                    Behavior on shadowVerticalOffset {
                        PropertyAnimation {
                            target: image_effect
                            property: "shadowVerticalOffset"
                            duration: 550
                            easing.type: Easing.InCubic
                        }
                    }
                }
            }
        }
    }
}