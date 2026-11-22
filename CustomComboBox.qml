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

        width: control.width - 4
        height: Config.common_element_height
        hoverEnabled: true

        background: Rectangle {
            radius: Config.common_element_radius - 2
            color: !control.alternate_colors
                ? (delegate.highlighted || delegate.hovered ? Config.secondary_cool_bg_color : Config.cool_bg_color)
                : (delegate.highlighted || delegate.hovered ? Config.secondary_cool_bg_color : Config.cool_bg_color)
        }

        contentItem: Text {
            leftPadding: 8
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
        anchors.rightMargin: 8
        fillMode: Image.PreserveAspectFit
        width: 13
        opacity: control.down || control.hovered ? 0.9 : 0.7
        source: "rc/images/expand_all.png"
    }

    contentItem: Text {
        leftPadding: 10
        rightPadding: 24

        text: control.displayText
        font: control.font
        color: Config.primary_text_color
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: control.width
        implicitHeight: Config.common_element_height
        radius: Config.common_element_radius
        border.width: 1
        border.color: !control.alternate_colors ? Config.secondary_cool_bg_color : Config.secondary_cool_bg_color
        color: !control.alternate_colors
            ? (control.pressed || control.down ? Config.secondary_cool_bg_color : Config.cool_bg_color)
            : (control.pressed || control.down ? Config.secondary_cool_bg_color : Config.cool_bg_color)
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        height: Math.min(contentItem.implicitHeight + 6, 220)
        padding: 3

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            spacing: 2
        }

        background: Rectangle {
            radius: Config.common_element_radius
            color: !control.alternate_colors ? Config.cool_bg_color : Config.cool_bg_color
            border.color: !control.alternate_colors ? Config.secondary_cool_bg_color : Config.secondary_cool_bg_color
            border.width: 1
        }
    }
}