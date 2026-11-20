import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Window
import Overlay
import QtGraphs

Window {
    id: overlay_window
    x:0
    y:0
    width: 800
    height: 400
    visible: true
    flags: Qt.NoDropShadowWindowHint | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
    color: "transparent"

    required property int process_resolution

    AudioCapture {
        id: audio
        resolution: process_resolution
    }

    Connections {
        target: audio
        function onUninitialized() {
            console.log("closing")
            overlay_window.close()
        }
    }

    function uninitialize() {
        audio.status = false
    }

    Rectangle{
        anchors.fill: parent
        color: "#2233226C"

        GraphsView {
            anchors.fill: parent
            theme : GraphsTheme {
                theme: GraphsTheme.Theme.UserDefined
                backgroundColor: "transparent"
                backgroundVisible: true
                seriesColors: ["green"]
                colorStyle: GraphsTheme.ColorStyle.Uniform
                labelFont.family: "Lucida Handwriting"
                labelFont.pointSize: 16
                gridVisible: false
                grid.mainColor: "ivory"
                grid.subColor: "gray"
                labelBackgroundColor: "black"
                labelBackgroundVisible: false
                labelBorderVisible: false
                labelTextColor: "white"
                multiHighlightColor: "green"
                singleHighlightColor: "darkRed"
                plotAreaBackgroundVisible: false
            }

            axisX: LogValueAxis {
                min: 1
                max: process_resolution/2
            }
            axisY: ValueAxis {
                min: 0
                max: process_resolution/2
            }

            LineSeries {
                // optimized: true
                id: line_series
                color: "#00ff00"
                GraphTransition {
                    GraphPointAnimation { duration: 24; easingCurve.type: Easing.Linear  }
                }
                XYModelMapper {
                    id: mapper
                    model: audio.model
                    series: line_series
                    orientation: Qt.Horizontal
                    ySection: 0
                    xSection: 1
                    count: process_resolution/2


                }
            }
        }
    }
}