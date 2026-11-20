import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import Overlay
import Config

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
                        console.log("deleting")
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
        color: "#3d4145"
        border.color: "cyan"
        visible: true



        Column {
            spacing: 24
            anchors.centerIn: parent

            CustomComboBox {
                id: resolution_combo_box
                visible: !app_loader.item
                alternate_colors: true
                font.family: Config.input_font
                width: 80
                anchors.horizontalCenter: parent.horizontalCenter
                model: ["1024", "2048", "4096"]
                onActivated: {
                    launcher_window.overlay_resolution = parseInt(currentText)
                }
            }

            Rectangle {
                width: 80
                height: 80
                anchors.horizontalCenter: parent.horizontalCenter
                radius: Config.common_container_radius
                color: "#3d4145"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (!app_loader.sourceComponent) {
                            app_loader.sourceComponent = overlay_component
                        } else if (app_loader.sourceComponent === overlay_component) {
                            // destroy but not close, overlay closes itself
                            app_loader.item.uninitialize()
                        }
                    }
                }
                Image {
                    source: "rc/images/CL.png"
                    anchors.fill: parent
                }
            }
        }
    }
}