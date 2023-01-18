import QtQuick 2.12
import QtQuick.Controls 2.12

import "." as NeroshopComponents // Triangle (in Triangle.qml)

ComboBox {
    id: control
    rightPadding: indicatorWidth
    property string baseColor: "#d3d3d3" // lightgrey
    property real radius: 5//2
    property real indicatorWidth: 50//30
    property string popupBorderColor: baseColor
    property string textColor: control.pressed ? "#101010" : "#383838"//"#17a81a" : "#21be2b"
    //property string ?Color: ""
    property bool indicatorDoNotPassBorder: false//(control.background.border.width == 0) ? false : true // This conditional statement makes no difference as it always defaults to true

    delegate: ItemDelegate {
        width: control.width
        contentItem: Text {
            text: control.textRole
                ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole])
                : modelData
            color: "#101010"//"#21be2b"
            font: control.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: control.highlightedIndex === index
    }

    indicator: Rectangle {
        id: indicatorRect
        width: indicatorWidth - Math.abs(children[0].anchors.leftMargin)
        height: (control.indicatorDoNotPassBorder) ? control.background.height - (control.background.border.width * 2) : control.background.height//8
        x: (control.indicatorDoNotPassBorder) ? control.width - this.width - control.background.border.width : control.width - this.width//control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        color: control.pressed ? "#4d426c" : "#605185"
        radius: control.background.radius
        //border.width: control.background.width; border.color: color
        ////Component.onCompleted: {console.log("indicator's actual width", width)}
        // Second rectangle to hide the left side of rect if rounded (has radius)
        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: -1 // This gives the indicator an extra width of 1
            width: parent.width / 2; height: parent.height
            color: parent.color
        }
        // Third rectangle to fill in the white gaps in the right-sided rounded edges      
        Rectangle {
            anchors.right: parent.right
            anchors.rightMargin: 0
            width: parent.width / 2; height: parent.height
            color: parent.color
            radius: parent.radius
        }                
        
        /*NeroshopComponents.Triangle {
            direction: Triangle.Direction.Down
            width: 20; height: width / 2
            anchors.centerIn: parent//anchors.horizontalCenter: parent.horizontalCenter
            anchors.leftMargin: parent.children[0].anchors.leftMargin
            fillColor: control.pressed ? "lightgray" : "#ffffff"
            strokeColor: this.fillColor
        }*/
        Text {
            anchors.centerIn: parent
            anchors.leftMargin: parent.children[0].anchors.leftMargin
            text: qsTr("\uf078")
            color: control.pressed ? "#d3d3d3" : "#ffffff"
            font.bold: true
        }
    }

    contentItem: TextField {
        leftPadding: 15//0
        //rightPadding: control.indicator.width + control.spacing
        readOnly: !control.editable

        text: control.displayText
        font: control.font
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
        //elide: Text.ElideRight
        background: Rectangle {
            color: control.baseColor
            radius: control.background.radius
        }
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        color: control.baseColor
        //border.width: control.visualFocus ? 2 : 1; border.color: color//control.pressed ? "yellow" : "orange"//"#17a81a" : "#21be2b"
        radius: control.radius
    }

    popup: Popup {
        y: control.height - 1
        width: control.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            border.color: control.popupBorderColor//"#21be2b"
            radius: (control.background.radius <= 5) ? control.background.radius : 5
        }
    }
}
