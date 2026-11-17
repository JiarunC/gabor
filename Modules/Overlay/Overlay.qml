import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Window
import Overlay
import QtGraphs

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

        GraphsView {
            anchors.fill: parent
            theme: GraphsTheme {
                colorScheme: GraphsTheme.ColorScheme.Dark
                seriesColors: ["#E0D080", "#B0A060"]
                borderColors: ["#807040", "#706030"]
                grid.mainColor: "#ccccff"
                grid.subColor: "#eeeeff"
                axisY.mainColor: "#ccccff"
                axisY.subColor: "#eeeeff"
            }
            axisX: BarCategoryAxis {
                lineVisible: false
            }
            axisY: ValueAxis {
                min: -10
                max: 10
            }
            BarSeries {
                barWidth: .1
                BarSet {
                    values: audio.gY
                }
            }
        }


    }


    property int common_padding: 6

}