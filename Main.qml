import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import Overlay

ApplicationWindow {
    id: launcher_window
    color: Config.bg_color
    minimumWidth: 720
    minimumHeight: 405
    maximumWidth: 720
    maximumHeight: 405
    visible: true
    title: qsTr("Launcher")

    Loader {
        id: app_loader
        sourceComponent: undefined
        onLoaded: {
            if (item) {
                launcher_window.visible = false
                item.closing.connect(function () {
                    app_loader.sourceComponent = undefined;
                    launcher_window.visible = true
                });
            }
        }
    }

    Component {
        Overlay {
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

        Row {
            spacing: 24
            anchors.centerIn: parent

            Rectangle {
                width: 50
                height: 50
                radius: Config.common_container_radius
                color: "#6393c3"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        app_loader.sourceComponent = overlay_component
                    }
                }
                //Image {
                //    source: "rc/images/GS2.png"
                //    anchors.fill: parent
                //}
            }
        }
    }
}