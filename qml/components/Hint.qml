// custom Tooltip with arrow
import QtQuick 2.12
import QtQuick.Controls 2.12
import "." as NeroshopComponents // Triangle (in Triangle.qml)

ToolTip {
    id: hint
    text: ""//qsTr("A descriptive tool tip of what the button does")
    visible: false
    implicitWidth: contentWidth + 20; implicitHeight: contentHeight + 20 //width: (text.length * 10) + 20// assuming each character is 10 pixels in width
    //x: parent.x + ((parent.width - this.width) / 2) //anchors.left: parent.left; anchors.leftMargin: (parent.width - this.width) / 2
    //y: (parent.y + parent.height) + 5 //anchors.top: parent.bottom; anchors.topMargin: 5
    delay: 500 // shows tooltip after hovering over it for 0.5 seconds
    property string direction: "up"//"down"
    property alias pointer: triangleTip
    property alias textObject: hintText
    property alias rect: hint.background
    property string textColor: "#ffffff"
    property string color: "#0a0a0a"
    readonly property alias backgroundColor: hint.color    
    property string borderColor: "#ffffff" 
    property real borderWidth: 0
    property real radius: 5//10

    contentItem: Text {
        id: hintText
        text: hint.text
        color: hint.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }

    background: Rectangle {
        color: hint.color
        opacity: 0.9
        border.color: hint.borderColor
        border.width: hint.borderWidth
        radius: hint.radius
        //gradient: "NightFade"
        // todo: Place triangle outside of the ToolTip. It is absolutely NOT suppose to be part of the ToolTip background. Both ToolTip and Triangle should be enclosed within an Item
        NeroshopComponents.Triangle {
            id: triangleTip
            // side: BottomCentered
            anchors.bottom: hint.bottom//anchors.top: hint.top
            //anchors.bottomMargin: 0
            anchors.horizontalCenter: hint.horizontalCenter
            width: 16; height: width / 2
            direction: Triangle.Direction.Down
            fillColor: "black"
            strokeColor: this.fillColor
        }       
    }
}
