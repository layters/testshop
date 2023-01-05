import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Layouts 1.12

import "." as NeroshopComponents

ProgressBar {
    id: progressBar
    value: 0.0////0.5 // 50 / 100 (50%)
    from: 0.0 // startValue
    to: 1.0 // endValue
    padding: 0//2
    property real radius: 0
    property string foregroundColor: "#17a81a"
    property string backgroundColor: "#e6e6e6" // Platinum
    property alias textObject: progressBarText
    property real barWidth: 300
    // static bar (background)
    background: Rectangle {
        implicitWidth: parent.barWidth // Implicit = the default if no width or height is supplied
        implicitHeight: 20//6
        color: progressBar.backgroundColor
        radius: progressBar.radius
    }
    // moving bar (foreground)
    contentItem: Item {
        implicitWidth: parent.barWidth
        implicitHeight: 20//4

        Rectangle {
            width: progressBar.visualPosition * parent.width
            height: parent.height
            radius: progressBar.radius//0// How to set radius on the left side only?
            color: progressBar.foregroundColor
        }
        // text
        Text {
            id: progressBarText
            text: (progressBar.value * 100).toString() + "%" // todo: for moneroDaemonSyncBar, display blocks (e.g 1509/16098674 blocks remaining)
            anchors.centerIn: parent
            visible: false
            //font.bold: true
            color: "#000000"
        }        
    }
}
