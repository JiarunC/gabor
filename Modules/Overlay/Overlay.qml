import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Window
import Overlay

Window {
    id: overlay_window
    width: 800
    height: 400
    visible: true
    flags: Qt.NoDropShadowWindowHint | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
    color: "transparent"

    AudioCapture {
        id: audio
    }

    Rectangle{
        anchors.fill: parent
        color: "#2233226C"
    }


    property int common_padding: 6

}