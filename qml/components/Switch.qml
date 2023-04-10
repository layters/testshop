import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

import "." as NeroshopComponents

Switch {
    id: control
    checked: false
    implicitWidth: 56
    property real radius: 50
    property string backgroundCheckedColor: "#6b5b95"
    property string foregroundColor: "#ffffff"
    
    indicator: Rectangle {
        id: background
        implicitWidth: control.width
        implicitHeight: 28
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: parent.radius
        color: control.checked ? control.backgroundCheckedColor : "dimgray" // background color
        border.color: this.color//"#ffffff"
        border.width: 3
        opacity: !enabled ? 0.7 : 1

        Rectangle {
            id: foreground
            x: control.checked ? parent.width - width - 5 : 5
            y: (parent.height - this.height) / 2 // vertically-centered
            // anchors code is equivalent to the above code:
            //anchors.left: parent.left
            //anchors.leftMargin: control.checked ? parent.width - width - 5 : 5
            //anchors.top: parent.top
            //anchors.topMargin: (parent.height - this.height) / 2
            width: parent.height - 10
            height: width // height and width must ALWAYS be equal in order to form a perfect circle
            radius: parent.radius
            color: control.foregroundColor//control.checked ? "#ffffff" : "#000000" // foreground color
            border.color: this.color 
            opacity: parent.opacity
        }
    }
}
