// styling by Gemini (Clearly)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Window
import QtQuick.Shapes
import Overlay
import QtGraphs

Window {
    id: overlay_window
    x: 0
    y: 0
    width: Screen.width
    height: Screen.height
    visible: true
    flags: Qt.Tool | Qt.NoDropShadowWindowHint | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
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

    GraphsView {
        id: graphs_view
        anchors.fill: parent
        marginTop: 0
        marginBottom: 0
        marginLeft: 0
        marginRight: 0

        theme: GraphsTheme {
            theme: GraphsTheme.Theme.UserDefined
            backgroundColor: "transparent"
            backgroundVisible: false
            plotAreaBackgroundColor: "transparent"
            plotAreaBackgroundVisible: false
            gridVisible: false
            labelsVisible: false
            labelBackgroundVisible: false
            labelBorderVisible: false
            colorStyle: GraphsTheme.ColorStyle.Uniform
        }

        axisX: LogValueAxis {
            min: 20
            max: 20000
            base: 10
            visible: false
            lineVisible: false
            labelsVisible: false
            gridVisible: false
            subGridVisible: false
            titleVisible: false
        }

        axisY: ValueAxis {
            min: 0
            max: 80
            visible: false
            lineVisible: false
            labelsVisible: false
            gridVisible: false
            subGridVisible: false
            titleVisible: false
        }

        AreaSeries {
            id: area_series
            borderColor: "#c8eefaff"
            borderWidth: 2.5

            gradient: LinearGradient {
                x1: 0
                y1: 0
                x2: 0
                y2: graphs_view.height
                GradientStop { position: 0.0; color:  "#1000ffff" }
                GradientStop { position: 0.60; color: "#1a00fff7" }
                GradientStop { position: 0.70; color: "#2a00ffb2" }
                GradientStop { position: 0.85; color: "#3a00ff80" }
                GradientStop { position: 1.0; color:  "#5a00eaff" }
            }

            upperSeries: SplineSeries {
                id: line_series
                onPointsAdded: {
                    line_series.pointsReplaced()
                }
                XYModelMapper {
                    id: mapper
                    model: audio.model
                    series: line_series
                    orientation: Qt.Horizontal
                    ySection: 0
                    xSection: 1
                }
            }

            lowerSeries: LineSeries {
                XYPoint {
                    x: 20
                    y: -20
                }

                XYPoint {
                    x: 20000
                    y: -20
                }
            }
        }
    }
}