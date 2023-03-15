import QtQuick 2.12
import QtQuick.Controls 2.12

import FontAwesome 1.0

import "." as NeroshopComponents

CheckBox {
    id: control
    //text: qsTr("")
    checked: false
    width: 20; height: 20
    property real radius: 3
    property string borderColor: control.checked ? "#4d426c" : "#989999"
    enum ShapeType {
        Check = 0, Block
    }
    property int shapeType: NeroshopComponents.CheckBox.ShapeType.Check
    property string textColor: control.down ? "#443a5f" : "#4d426c"
        
    indicator: Rectangle { // This is the actual visual CheckBox or its background, you could say
        anchors.fill: parent
        radius: control.radius
        border.color: control.borderColor

        Rectangle {
            width: parent.width / 2; height: width
            anchors.centerIn: parent
            radius: 2
            color: control.down ? "#443a5f" : "#4d426c"
            visible: control.checked && (control.shapeType == NeroshopComponents.CheckBox.ShapeType.Block)
        }
        // or
        Text {
            text: qsTr("\uf00c")
            font.pointSize: control.width / 2
            font.bold: true
            font.family: FontAwesome.fontFamily
            anchors.centerIn: parent
            color: control.down ? "#443a5f" : "#4d426c"
            visible: control.checked && (control.shapeType == NeroshopComponents.CheckBox.ShapeType.Check)
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
}
