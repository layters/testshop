import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

import "." as NeroshopComponents

Switch {
    id: control
    text: (this.checked) ? qsTr("ON") : qsTr("OFF");
    checked: !NeroshopComponents.Style.darkTheme
    onClicked: {
        if(this.checked === true) {
            NeroshopComponents.Style.darkTheme = false
        }
        else if(this.checked === false) {
            NeroshopComponents.Style.darkTheme = true
        }
        settingsDialog.theme.displayText = (NeroshopComponents.Style.darkTheme) ? settingsDialog.theme.lastUsedDarkTheme : settingsDialog.theme.lastUsedLightTheme // correct the themeName
        NeroshopComponents.Style.themeName = settingsDialog.theme.displayText // update the actual theme (name)
    }    
    implicitWidth: 56//64//72//88
    
    indicator: Rectangle {
        id: background
        implicitWidth: control.width//88//48
        implicitHeight: 28//36//44//50//26
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 50//13
        color: "transparent"//control.checked ? "black" : "white"//control.checked ? "#6b5b95" : "dimgray" // background color
        border.color: control.checked ? "#ffc408" : "black"//control.checked ? /*"#6b5b95"*/ : "#cccccc"
        border.width: 3

        Rectangle {
            id: foreground
            x: control.checked ? parent.width - width - 8: 8
            y: (parent.height - this.height) / 2
            // anchors code is equivalent to the above code:
            //anchors.left: parent.left
            //anchors.leftMargin: control.checked ? parent.width - width - 8 : 8
            //anchors.top: parent.top
            //anchors.topMargin: (parent.height - this.height) / 2
            width: parent.height - 15//26
            height: width
            radius: parent.radius//50//13
            color: control.checked ? "#ffc408" : "black"//control.down ? "#cccccc" : "#ffffff" // foreground color
            border.color: parent.border.color//control.checked ? "#6b5b95"/*(control.down ? "#6b5b95" : "#6b5b95")*/ : "#999999"
        
            /*Image {
                id: theme_icon
                source: control.checked ? neroshopResourceDir + "/sun.png" : neroshopResourceDir + "/moon.png"
                width: parent.width / 2; height: parent.height / 2//width: 16; height: 16
                //fillMode: Image.PreserveAspectFit //<= doesn't work (￣ω￣;)
                //horizontalAlignment: Image.AlignHCenter; verticalAlignment: Image.AlignVCenter //<= doesn't work (￣^￣)
                anchors.centerIn: parent
            }
            ColorOverlay {
                anchors.fill: theme_icon
                source: theme_icon
                color: background.color//control.checked ? "#ea9300" : background.color
                visible: theme_icon.visible
            }*/       
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.checked ? "#6b5b95" : "dimgray"
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
        visible: false // hide text
    }
}
