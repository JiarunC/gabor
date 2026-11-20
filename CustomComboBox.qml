import QtQuick
import QtQuick.Controls.Basic
import Config

ComboBox {
    id: control

    property bool alternate_colors: false

    delegate: ItemDelegate {
        id: delegate

        required property var model
        required property int index

        width: control.width
        background: Rectangle {

            color: !alternate_colors
                ? (delegate.highlighted ? Config.secondary_warm_bg_color : Config.warm_bg_color)
                : (delegate.highlighted ? Config.secondary_cool_bg_color : Config.cool_bg_color)
        }


        contentItem: Text {
            text: delegate.model[control.textRole]
            color: Config.primary_text_color
            font: control.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: control.highlightedIndex === index
    }

    indicator: Image {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 6
        fillMode: Image.PreserveAspectFit
        width: 14
        source: "rc/images/expand_all.png"
    }

    contentItem: Text {
        leftPadding: 6
        // rightPadding: control.indicator.width + control.spacing

        text: control.displayText
        font: control.font
        color: Config.primary_text_color
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: control.width
        implicitHeight: Config.common_element_height
        color: !alternate_colors
            ? (control.pressed ? Config.secondary_warm_bg_color : Config.warm_bg_color)
            : (control.pressed ? Config.secondary_cool_bg_color : Config.cool_bg_color)

        // border.width: control.visualFocus ? 2 : 1
        radius: Config.common_element_radius
    }

    popup: Popup {
        y: control.height - 1
        width: control.width
        height: Math.min(contentItem.implicitHeight, 220)
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
        }

        background: Rectangle {
            border.color: !alternate_colors ? "#8a5b85" : "#00b4d8"
            border.width: 2
            radius: 2
        }
    }
}