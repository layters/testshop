// custom Tooltip with arrow
import QtQuick 2.12
import QtQuick.Controls 2.12
import "." as NeroshopComponents // Triangle (in Triangle.qml)

ToolTip {
    id: hint
    text: ""//"position: " + tooltip_arrow.position + " (" + tooltip_arrow.offsetX + ")"//qsTr("A descriptive tool tip of what the button does")
    visible: false//true//false
    width: contentWidth + 20; height: 50 //width: (text.length * 10) + 20// assuming each character is 10 pixels in width
    //x: parent.x + ((parent.width - this.width) / 2) //anchors.left: parent.left; anchors.leftMargin: (parent.width - this.width) / 2
    //y: (parent.y + parent.height) + 5 //anchors.top: parent.bottom; anchors.topMargin: 5
    delay: 500 // shows tooltip after hovering over it for 0.5 seconds
    property string direction: "up"//"down"

    contentItem: Text {
        text: hint.text
        font: hint.font
        color: "#ffffff"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: "#000000"// #0a0a0a = 10, 10, 10
        opacity: 0.9
        border.color: "#ffffff"
        border.width: 0
        radius: 5//10
        //gradient: "NightFade"
        NeroshopComponents.Triangle {
            //anchors.left: hint.left//hint.horizontalCenter
            //anchors.leftMargin: 100
            //anchors.top: parent.top//hint.top
            //anchors. :
                id: pointer
                //x: this.x
                //y: this.y
                ////direction: hint.direction
                color: "black"
        }       
    }
}
