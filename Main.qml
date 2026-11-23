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

                Button {
                    id: start_button
                    width: 88
                    height: 88
                    anchors.horizontalCenter: parent.horizontalCenter
                    hoverEnabled: true

                    property bool isActive: launcher_window.overlay_instance !== null
                    property real wavePhase: 0.0
                    property real ripplePhase: 0.0

                    Shortcut {
                        sequence: "Ctrl+Alt+A"
                        onActivated: launcher_window.toggleOverlay()
                    }

                    onClicked: launcher_window.toggleOverlay()

                    NumberAnimation on wavePhase {
                        from: 0
                        to: Math.PI * 2
                        duration: start_button.isActive ? 900 : (start_button.hovered ? 1400 : 2600)
                        loops: Animation.Infinite
                        running: true
                    }

                    NumberAnimation on ripplePhase {
                        from: 0.0
                        to: 1.0
                        duration: start_button.isActive ? 1100 : 2000
                        loops: Animation.Infinite
                        running: true
                    }

                    onWavePhaseChanged: wave_canvas.requestPaint()

                    scale: start_button.down ? 0.91 : (start_button.hovered ? 1.07 : 1.0)
                    Behavior on scale {
                        NumberAnimation { duration: 220; easing.type: Easing.OutBack }
                    }

                    contentItem: Item {
                        anchors.fill: parent

                        // Pulsing sonar shockwave ring 1
                        Rectangle {
                            anchors.centerIn: parent
                            width: 64 + start_button.ripplePhase * 34
                            height: width
                            radius: width / 2
                            color: "transparent"
                            border.width: start_button.isActive ? 2 : 1.2
                            border.color: start_button.isActive ? Config.vibrant_cool_text_color : Config.secondary_cool_bg_color
                            opacity: (1.0 - start_button.ripplePhase) * (start_button.isActive ? 0.85 : (start_button.hovered ? 0.55 : 0.25))
                        }

                        // Pulsing sonar shockwave ring 2 (phase-offset by 0.5)
                        Rectangle {
                            property real phase2: (start_button.ripplePhase + 0.5) % 1.0
                            anchors.centerIn: parent
                            width: 64 + phase2 * 34
                            height: width
                            radius: width / 2
                            color: "transparent"
                            border.width: start_button.isActive ? 1.5 : 1
                            border.color: start_button.isActive ? Config.primary_text_color : Config.vibrant_cool_text_color
                            opacity: (1.0 - phase2) * (start_button.isActive ? 0.65 : (start_button.hovered ? 0.4 : 0.15))
                        }

                        // Outer rotating diamond/chamfered frame
                        Rectangle {
                            id: outer_gyro
                            anchors.centerIn: parent
                            width: 66
                            height: 66
                            radius: 16
                            color: "transparent"
                            border.width: 1.5
                            border.color: start_button.isActive || start_button.hovered
                                ? Config.vibrant_cool_text_color
                                : Config.secondary_cool_bg_color
                            opacity: start_button.isActive ? 0.9 : (start_button.hovered ? 0.75 : 0.45)

                            RotationAnimator on rotation {
                                from: 0
                                to: 360
                                duration: start_button.isActive ? 4000 : 10000
                                loops: Animation.Infinite
                                running: true
                            }

                            // Corner orbital nodes
                            Repeater {
                                model: 4
                                Rectangle {
                                    required property int index
                                    width: 5
                                    height: 5
                                    radius: 2.5
                                    color: start_button.isActive ? Config.primary_text_color : Config.vibrant_cool_text_color
                                    x: (index % 2 === 0) ? -2.5 : outer_gyro.width - 2.5
                                    y: (index < 2) ? -2.5 : outer_gyro.height - 2.5
                                }
                            }
                        }

                        // Counter-rotating inner octagonal frame
                        Rectangle {
                            id: inner_gyro
                            anchors.centerIn: parent
                            width: 66
                            height: 66
                            radius: 16
                            rotation: 45
                            color: "transparent"
                            border.width: 1.5
                            border.color: start_button.isActive
                                ? Config.primary_text_color
                                : (start_button.hovered ? Config.vibrant_cool_text_color : Config.secondary_cool_bg_color)
                            opacity: start_button.isActive ? 0.75 : (start_button.hovered ? 0.6 : 0.3)

                            RotationAnimator on rotation {
                                from: 45
                                to: -315
                                duration: start_button.isActive ? 5500 : 12000
                                loops: Animation.Infinite
                                running: true
                            }
                        }

                        // Core circular reactor / wave chamber
                        Rectangle {
                            id: core_circle
                            anchors.centerIn: parent
                            width: 58
                            height: 58
                            radius: 29
                            color: Qt.darker(Config.cool_bg_color, 1.8)
                            border.width: 2
                            border.color: start_button.isActive
                                ? Config.vibrant_cool_text_color
                                : (start_button.hovered ? Config.primary_text_color : Config.secondary_cool_bg_color)

                            Behavior on border.color {
                                ColorAnimation { duration: 200 }
                            }

                            // Live multi-harmonic wave & circular spectrum Canvas
                            Canvas {
                                id: wave_canvas
                                anchors.fill: parent
                                anchors.margins: 3

                                onPaint: {
                                    var ctx = getContext("2d");
                                    var w = width;
                                    var h = height;
                                    var cx = w / 2;
                                    var cy = h / 2;
                                    var r = Math.min(w, h) / 2 - 1;

                                    ctx.clearRect(0, 0, w, h);
                                    ctx.save();

                                    // Clip to circular core
                                    ctx.beginPath();
                                    ctx.arc(cx, cy, r, 0, Math.PI * 2);
                                    ctx.clip();

                                    var p = start_button.wavePhase;
                                    var amp = start_button.isActive ? 9.0 : (start_button.hovered ? 6.5 : 3.5);
                                    var baseLine = start_button.isActive ? cy + 2 : cy + 6;

                                    // Deep wave layer 1 (cool_bg_color)
                                    ctx.beginPath();
                                    ctx.moveTo(0, h);
                                    for (var x1 = 0; x1 <= w; x1 += 2) {
                                        var y1 = baseLine + Math.sin((x1 / w) * Math.PI * 2.5 + p) * amp
                                                          + Math.cos((x1 / w) * Math.PI * 4.0 - p * 1.5) * (amp * 0.4);
                                        ctx.lineTo(x1, y1);
                                    }
                                    ctx.lineTo(w, h);
                                    ctx.closePath();
                                    ctx.fillStyle = Config.cool_bg_color;
                                    ctx.fill();

                                    // Mid wave layer 2 (secondary_cool_bg_color)
                                    ctx.beginPath();
                                    ctx.moveTo(0, h);
                                    for (var x2 = 0; x2 <= w; x2 += 2) {
                                        var y2 = baseLine + 3 + Math.sin((x2 / w) * Math.PI * 3.0 - p * 1.3 + 1.2) * (amp * 0.9)
                                                              + Math.sin((x2 / w) * Math.PI * 5.0 + p * 2.0) * (amp * 0.35);
                                        ctx.lineTo(x2, y2);
                                    }
                                    ctx.lineTo(w, h);
                                    ctx.closePath();
                                    ctx.fillStyle = Config.secondary_cool_bg_color;
                                    ctx.fill();

                                    // Foreground crest wave layer 3 (vibrant_cool_text_color + ivory foam crest)
                                    ctx.beginPath();
                                    ctx.moveTo(0, h);
                                    for (var x3 = 0; x3 <= w; x3 += 2) {
                                        var y3 = baseLine + 7 + Math.cos((x3 / w) * Math.PI * 2.2 + p * 1.8) * (amp * 0.85);
                                        ctx.lineTo(x3, y3);
                                    }
                                    ctx.lineTo(w, h);
                                    ctx.closePath();
                                    ctx.fillStyle = start_button.isActive || start_button.hovered
                                        ? Config.vibrant_cool_text_color
                                        : Config.secondary_cool_bg_color;
                                    ctx.fill();

                                    // Foam crest stroke on wave 2
                                    ctx.beginPath();
                                    for (var x4 = 0; x4 <= w; x4 += 2) {
                                        var y4 = baseLine + 3 + Math.sin((x4 / w) * Math.PI * 3.0 - p * 1.3 + 1.2) * (amp * 0.9)
                                                              + Math.sin((x4 / w) * Math.PI * 5.0 + p * 2.0) * (amp * 0.35);
                                        if (x4 === 0) ctx.moveTo(x4, y4);
                                        else ctx.lineTo(x4, y4);
                                    }
                                    ctx.strokeStyle = Config.primary_text_color;
                                    ctx.lineWidth = start_button.isActive ? 1.8 : 1.2;
                                    ctx.stroke();

                                    // Radial audio-spectrum ticks around inner perimeter when hovered or active
                                    if (start_button.isActive || start_button.hovered) {
                                        var numBars = 24;
                                        for (var i = 0; i < numBars; i++) {
                                            var angle = (i / numBars) * Math.PI * 2 + p * 0.25;
                                            var barLen = 2.5 + Math.abs(Math.sin(i * 1.7 + p * 2.5)) * (start_button.isActive ? 6.5 : 4.0);
                                            var rOut = r - 1;
                                            var rIn = rOut - barLen;
                                            ctx.beginPath();
                                            ctx.moveTo(cx + Math.cos(angle) * rOut, cy + Math.sin(angle) * rOut);
                                            ctx.lineTo(cx + Math.cos(angle) * rIn, cy + Math.sin(angle) * rIn);
                                            ctx.strokeStyle = (i % 2 === 0) ? Config.primary_text_color : Config.vibrant_cool_text_color;
                                            ctx.lineWidth = 1.4;
                                            ctx.stroke();
                                        }
                                    }

                                    ctx.restore();
                                }
                            }

                            // Center label / icon badge
                            Text {
                                anchors.centerIn: parent
                                font.family: Config.info_font
                                font.pixelSize: start_button.isActive ? 11 : 13
                                font.bold: true
                                font.letterSpacing: 1.5
                                text: start_button.isActive ? qsTr("LIVE") : qsTr("WAV")
                                color: Config.primary_text_color
                            }
                        }
                    }

                    background: Item {
                        anchors.fill: parent

                        // Drop shadow core
                        Rectangle {
                            id: shadow_caster
                            anchors.centerIn: parent
                            width: 58
                            height: 58
                            radius: 29
                            color: Config.bg_color
                            visible: false
                        }

                        MultiEffect {
                            source: shadow_caster
                            anchors.centerIn: shadow_caster
                            width: shadow_caster.width
                            height: shadow_caster.height
                            shadowEnabled: true
                            shadowColor: "black"
                            shadowOpacity: 0.75
                            shadowVerticalOffset: start_button.down ? 2 : (start_button.hovered ? 5 : 10)
                            shadowScale: 1.05

                            Behavior on shadowVerticalOffset {
                                NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }
        }
    }
}