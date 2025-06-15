import QtQuick 2.15
import QtQuick.Controls 2.15

import "." as NeroshopComponents // Style

Item {
    id: root
    width: childrenRect.width
    height: childrenRect.height

    property var notificationQueue: []
    property bool isShowing: false

    Rectangle {
        id: popup
        width: Math.min(250, message.contentWidth + 100)
        height: 50
        color: (NeroshopComponents.Style.darkTheme) ? "#CCC" : "#333"
        radius: 8
        opacity: 0
        visible: opacity > 0

        Text {
            id: message
            anchors.centerIn: parent
            color: (NeroshopComponents.Style.darkTheme) ? "black" : "white"
            font.pointSize: 10
            text: ""
            //onTextChanged: console.log("message.contentWidth",message.contentWidth)
        }
    }

    Timer {
        id: displayTimer
        interval: 3000 // visible duration
        repeat: false
        onTriggered: fadeOut()
    }

    function showNotification(message) {
        notificationQueue.push(message)
        if (!isShowing) {
            showNext()
        }
    }

    function showNext() {
        if (notificationQueue.length === 0) {
            isShowing = false
            return
        }
        isShowing = true
        message.text = notificationQueue.shift()
        fadeIn()
    }

    function fadeIn() {
        popup.opacity = 0
        popup.visible = true
        fadeInAnim.start()
    }

    function fadeOut() {
        fadeOutAnim.start()
    }
    
    PropertyAnimation {
        id: fadeInAnim
        target: popup
        property: "opacity"
        from: 0
        to: 1
        duration: 500
        easing.type: Easing.InOutQuad
        onStopped: {
            // After fade-in, wait some time then fade out
            displayTimer.start()
        }
    }

    PropertyAnimation {
        id: fadeOutAnim
        target: popup
        property: "opacity"
        from: 1
        to: 0
        duration: 500
        easing.type: Easing.InOutQuad
        onStopped: {
            popup.visible = false
            showNext()
        }
    }
}
