import QtQuick 2.15
import QtQuick.Controls 2.15

import FontAwesome 1.0

import "." as NeroshopComponents // Style

Item {
    id: root
    width: childrenRect.width
    height: childrenRect.height

    property var notificationQueue: []
    property bool isShowing: false
    
    enum ToastType {
        Normal = 0, Err = 1, Warn = 2, Success = 3, Info = 4
    }
    property int toastType: NeroshopComponents.Toast.ToastType.Normal

    Rectangle {
        id: popup
        width: Math.min(250, icon.implicitWidth + icon.anchors.leftMargin + message.implicitWidth + message.anchors.leftMargin + 100)//message.contentWidth + 100)
        height: 50
        color: { 
            switch (toastType) {
                case NeroshopComponents.Toast.ToastType.Err: return "#FFC9C6"
                case NeroshopComponents.Toast.ToastType.Warn: return "#FFFFE0"
                case NeroshopComponents.Toast.ToastType.Success: return "#B9EAB3"
                case NeroshopComponents.Toast.ToastType.Info: return "#AEC6CF"
                default: return ((NeroshopComponents.Style.darkTheme) ? "#CCC" : "#333")
            }
        }
        radius: 8
        opacity: 0
        visible: opacity > 0
        
        Text {
            id: icon
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            font.bold: true
            font.family: FontAwesome.fontFamily
            font.pixelSize: 20
            color: {
                switch (toastType) {
                    case NeroshopComponents.Toast.ToastType.Err: return "#ff6347"
                    case NeroshopComponents.Toast.ToastType.Warn: return "gold"//"#ffff00"
                    case NeroshopComponents.Toast.ToastType.Success: return "#7A9B26"//"#6b8e23"//"#9acd32"
                    case NeroshopComponents.Toast.ToastType.Info: return "#1e90ff"
                    default: return ((NeroshopComponents.Style.darkTheme) ? "black" : "white")
                }
            }
            visible: toastType !== NeroshopComponents.Toast.ToastType.Normal
            text: {
                switch (toastType) {
                    case NeroshopComponents.Toast.ToastType.Err: return qsTr("\uf057")  // 'times-circle'
                    case NeroshopComponents.Toast.ToastType.Warn: return qsTr("\uf071") // 'exclamation-triangle'
                    case NeroshopComponents.Toast.ToastType.Success: return qsTr("\uf058") // 'check-circle'
                    case NeroshopComponents.Toast.ToastType.Info: return qsTr("\uf05a") // 'info-circle'
                    default: return ""
                }
            }
        }

        Text {
            id: message
            anchors.verticalCenter: icon.verticalCenter
            anchors.left: icon.visible ? icon.right : undefined
            anchors.leftMargin: icon.visible ? 8 : 0
            anchors.horizontalCenter: icon.visible ? undefined : parent.horizontalCenter
            color: icon.visible ? "black" : ((NeroshopComponents.Style.darkTheme) ? "black" : "white")
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
