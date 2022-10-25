import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Item {
    TextField {
        id: search_bar
        color: "#ffffff" // textColor
        width: 400; height: 40
        selectByMouse: true
        placeholderText: qsTr("Search")
        
        background: Rectangle { 
            color: "#050506"
            radius: 5
        }
    }

    Button {
        id: search_button
        text: qsTr("Search")
        //onClicked: 
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        anchors.left: search_bar.right
        anchors.leftMargin: 5//1
        anchors.top: search_bar.top
        width: 50; height: search_bar.height
        
        icon.source: "qrc:/images/search.png"//neroshopResourceDir + "/search.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "#8071a8"//"#40404f"
            radius: search_bar.background.radius
        }     
        
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                parent.background.color = "#66578e"
            }
            onExited: {
                parent.background.color = "#8071a8"
            }
        } 
    }
}
