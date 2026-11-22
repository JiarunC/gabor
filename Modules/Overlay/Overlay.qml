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
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    visible: true
    flags: Qt.NoDropShadowWindowHint | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
    color: "transparent"

    required property int process_resolution
    property int process_interval
    property int process_target_points

    AudioCapture {
        id: audio
        resolution: process_resolution
        updateInterval: process_interval
        nTargetPoints: process_target_points
    }

    Connections {
        target: audio
        function onUninitialized() {
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
                min: 20
                max: 20000
                base: 10
            }
            axisY: ValueAxis {
                min: 0
                max: 80
            }

            AreaSeries {
                id: area_series
                color: "#4000ff00"       // Semi-transparent green fill
                borderColor: "#00ff00"   // Solid green top border
                borderWidth: 2

                upperSeries: LineSeries {
                    // optimized: true
                    id: line_series
                    XYModelMapper {
                        id: mapper
                        model: audio.model
                        series: line_series
                        orientation: Qt.Horizontal
                        ySection: 0
                        xSection: 1
                        count: 200
                    }
                }
            }
        }
    }
}