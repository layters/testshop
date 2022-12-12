import QtQuick 2.12
import QtQuick.Controls 2.12

RadioButton {
    id: control
    text: qsTr("RadioButton")
    checked: true
    property string color: control.down ? "#39304f" : "#6b5b95"
    property string innerColor: borderColor//"white"
    property string borderColor: control.down ? "#39304f" : "#6b5b95"
    property string textColor: control.down ? "#39304f" : "#6b5b95"
    property alias textObject: radioButtonText

    indicator: Rectangle {
        implicitWidth: 16//26
        implicitHeight: 16//26
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 13
        color: innerColor
        border.color: control.borderColor

        Rectangle {
            width: 10//14
            height: 10//14
            anchors.horizontalCenter: parent.horizontalCenter////x: 5//6
            anchors.verticalCenter: parent.verticalCenter////y: 5//6
            radius: 7
            color: control.color
            visible: control.checked
        }
    }

    contentItem: Text {
        id: radioButtonText
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
}
